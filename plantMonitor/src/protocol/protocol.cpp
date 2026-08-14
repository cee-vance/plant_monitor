#include "src/globals.h"
#include "src/protocol.h"
#include "src/actuators.h"
#include <Arduino.h>



volatile unsigned long lastDebounceUp   = 0;
volatile unsigned long lastDebounceDown = 0;
volatile unsigned long lastDebounceEnter= 0;





static TimerHandle_t schedule_timer = NULL;


void protocol_apply_schedule(const config_msg_t &cfg) {

    // If user selected THRESHOLD mode → stop schedule timer
    if (cfg.mode == MODE_THRESHOLD) {
        if (schedule_timer != NULL) {
            xTimerStop(schedule_timer, 0);
        }
        return;
    }

    // Otherwise MODE_SCHEDULE → create/start timer
    uint32_t period_ms = cfg.schedule_mins * 60UL * 1000UL;

    // Create timer if needed
    if (schedule_timer == NULL) {
        schedule_timer = xTimerCreate(
            "sched",
            pdMS_TO_TICKS(period_ms),
            pdTRUE,        // auto-reload
            NULL,
            schedule_timer_callback
        );
    } else {
        // Timer already exists → update period
        xTimerChangePeriod(schedule_timer,
                           pdMS_TO_TICKS(period_ms),
                           0);
    }

    // Start the timer
    xTimerStart(schedule_timer, 0);

    // Optional LCD feedback
    // lcd.clear();
    // lcd.print("Schedule: ");
    // lcd.print(cfg.schedule_mins);
    // lcd.print(" mins");
    // vTaskDelay(1500 / portTICK_PERIOD_MS);
}





void  notify_mode_manager(uint32_t notifyBit) {

    if (ui_mode == UI_MODE_LOGGING) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xTaskNotifyFromISR(mode_mgr_handle,
                           notifyBit,
                           eSetBits,
                           &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}



void IRAM_ATTR isr_up() {
    if (debounce(BTN_UP_ID, BTN_UP)) {
        if (ui_mode == UI_MODE_LOGGING){
            notify_mode_manager(UI_NOTIFY_UP);
        }
    }
}

void IRAM_ATTR isr_down() {
    if (debounce(BTN_DOWN_ID, BTN_DOWN)) {
        if (ui_mode == UI_MODE_LOGGING){
            notify_mode_manager(UI_NOTIFY_DOWN);
        }
    }
}

void IRAM_ATTR isr_enter() {
    if (debounce(BTN_ENTER_ID, BTN_ENTER)) {
          if (ui_mode == UI_MODE_LOGGING){
            notify_mode_manager(UI_NOTIFY_ENTER);
        }
    }
}






void mode_mgr_task(void* arg) {

    uint32_t notifyValue = 0;

    while (true) {

        // Wait for ISR notification (button press)
        xTaskNotifyWait(
            0,
            ULONG_MAX,
            &notifyValue,
            portMAX_DELAY
        );

        // Only one transition: LOGGING → CONFIG
        if (ui_mode == UI_MODE_LOGGING) {

            Serial.println("UI MODE CHANGING!! LFG!!");

            ui_mode = UI_MODE_CONFIG;

            // Suspend logging tasks
            vTaskSuspend(regularDisplayTaskHandle);
            vTaskSuspend(read_sensors_handle);
            vTaskSuspend(displayNowTaskHandle);
            
            vTaskDelay(100 / portTICK_PERIOD_MS);

            // Run synchronous wizard
           run_config_wizard();

            ui_mode = UI_MODE_LOGGING;

        // NOW apply schedule safely
        protocol_apply_schedule(global_config);

        vTaskDelay(100 / portTICK_PERIOD_MS);
        vTaskResume(regularDisplayTaskHandle);
        vTaskResume(read_sensors_handle);
        vTaskResume(displayNowTaskHandle);

        attach_buttons();

      

            continue;
        }
    }
}



 bool  debounce(button_id_t id, int gpioPin) {

    unsigned long now = millis();
    volatile unsigned long* lastTime;

    // Select correct timestamp
    switch (id) {
        case BTN_UP_ID:    lastTime = &lastDebounceUp; break;
        case BTN_DOWN_ID:  lastTime = &lastDebounceDown; break;
        case BTN_ENTER_ID: lastTime = &lastDebounceEnter; break;
    }

    // Check timing and pin state
    if ((now - *lastTime) > DEBOUNCE_MS &&
        digitalRead(gpioPin) == LOW) {

        *lastTime = now;
        return true;
    }

    return false;
}


void attach_buttons() {
    attachInterrupt(BTN_UP,    isr_up,    FALLING);
    attachInterrupt(BTN_DOWN,  isr_down,  FALLING);
    attachInterrupt(BTN_ENTER, isr_enter, FALLING);
}

void detach_buttons() {
    detachInterrupt(BTN_UP);
    detachInterrupt(BTN_DOWN);
    detachInterrupt(BTN_ENTER);
}


button_id_t wait_for_button() {
    const int debounce_ms = 40;
    const int release_ms  = 40;

    while (true) {
        if (digitalRead(BTN_UP) == LOW) {
            delay(debounce_ms);
            if (digitalRead(BTN_UP) == LOW) {
                while (digitalRead(BTN_UP) == LOW) {
                    delay(release_ms);
                }
                return BTN_UP_ID;
            }
        }

        if (digitalRead(BTN_DOWN) == LOW) {
            delay(debounce_ms);
            if (digitalRead(BTN_DOWN) == LOW) {
                while (digitalRead(BTN_DOWN) == LOW) {
                    delay(release_ms);
                }
                return BTN_DOWN_ID;
            }
        }

        if (digitalRead(BTN_ENTER) == LOW) {
            delay(debounce_ms);
            if (digitalRead(BTN_ENTER) == LOW) {
                while (digitalRead(BTN_ENTER) == LOW) {
                    delay(release_ms);
                }
                return BTN_ENTER_ID;
            }
        }

        delay(10);
    }
}


bool prompt_confirm(const char* message) {
    lcd.clear();
    lcd.print(message);
    lcd.setCursor(0, 1);
    lcd.print("UP=yes DOWN=no");

    button_id_t b = wait_for_button();

    return (b == BTN_UP_ID || b == BTN_ENTER_ID);
}


void run_config_wizard() {
    detach_buttons();   // stop ISR-driven UI

    // 1. Confirm entering config mode
    if (!prompt_confirm("Enter config?")) {
        
        return;
    }

    // Build a temporary config from current settings
    config_msg_t temp = global_config;

    // 2. Prompt for watering mode (THRESHOLD or SCHEDULE)
    temp.mode = prompt_for_watering_mode(global_config.mode);

    // 3. Depending on mode, prompt for threshold or schedule hours
    if (temp.mode == MODE_THRESHOLD) {
        temp.threshold_pct =
            prompt_for_int("Threshold %:", global_config.threshold_pct, 0, 100);
    } else {
    temp.schedule_mins =
        prompt_for_int("Water mins:", global_config.schedule_mins, 0, 180);

    }

    // 4. Prompt for temperature range
    temp.temp_min =
        prompt_for_int("Temp min:", global_config.temp_min, 0, 50);

    temp.temp_max =
        prompt_for_int("Temp max:", global_config.temp_max, temp.temp_min+1, 50);

    // 5. Final confirmation

    if(prompt_confirm("Apply settings?")){
        
         global_config = temp;   // apply new config
   
    }

    return;
    
}



int prompt_for_int(const char* prompt, int initialValue, int minVal, int maxVal) {
    int value = initialValue;

    lcd.clear();
    lcd.print(prompt);
    lcd.setCursor(0, 1);
    lcd.print(value);

    while (true) {
        button_id_t b = wait_for_button();

        if (b == BTN_UP_ID) {
            value++;
            if (value > maxVal) value = maxVal;
        }
        else if (b == BTN_DOWN_ID) {
            value--;
            if (value < minVal) value = minVal;
        }
        else if (b == BTN_ENTER_ID) {
            return value;   // confirm
        }

        // Update display
        lcd.setCursor(0, 1);
        lcd.print("        ");   // clear line
        lcd.setCursor(0, 1);
        lcd.print(value);
    }
}



control_mode_t prompt_for_watering_mode(control_mode_t initialMode) {
    control_mode_t mode = initialMode;

    lcd.clear();
    lcd.print("Watering mode:");
    lcd.setCursor(0, 1);
    lcd.print(mode == MODE_THRESHOLD ? "THRESHOLD" : "SCHEDULE");

    while (true) {
        button_id_t b = wait_for_button();

        if (b == BTN_UP_ID || b == BTN_DOWN_ID) {
            // toggle mode
            mode = (mode == MODE_THRESHOLD) ? MODE_SCHEDULE : MODE_THRESHOLD;

            lcd.setCursor(0, 1);
            lcd.print("          "); // clear line
            lcd.setCursor(0, 1);
            lcd.print(mode == MODE_THRESHOLD ? "THRESHOLD" : "SCHEDULE");
        }
        else if (b == BTN_ENTER_ID) {
            return mode;   // confirm selection
        }
    }
}


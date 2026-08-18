// ======================================================
//  Includes
// ======================================================
#include "src/globals.h"
#include "src/protocol.h"
#include "src/actuators.h"
#include <Arduino.h>

// ======================================================
//  Debounce State
// ======================================================
volatile unsigned long lastDebounceUp    = 0;
volatile unsigned long lastDebounceDown  = 0;
volatile unsigned long lastDebounceEnter = 0;

// ======================================================
//  Schedule Timer
// ======================================================
static TimerHandle_t schedule_timer = NULL;

// ======================================================
// Name: protocol_apply_schedule
// Description: Applies watering schedule settings by
//              creating or updating a FreeRTOS timer.
// ======================================================
void protocol_apply_schedule(const config_msg_t &cfg) {

    if (cfg.mode == MODE_THRESHOLD) {
        if (schedule_timer != NULL) {
            xTimerStop(schedule_timer, 0);
        }
        return;
    }

    uint32_t period_ms = cfg.schedule_mins * 60UL * 1000UL;

    if (schedule_timer == NULL) {
        schedule_timer = xTimerCreate(
            "sched",
            pdMS_TO_TICKS(period_ms),
            pdTRUE,
            NULL,
            schedule_timer_callback
        );
    } else {
        xTimerChangePeriod(schedule_timer,
                           pdMS_TO_TICKS(period_ms),
                           0);
    }

    xTimerStart(schedule_timer, 0);
}

// ======================================================
// Name: notify_mode_manager
// Description: Sends UI notifications from ISR to the
//              mode manager task.
// ======================================================
void notify_mode_manager(uint32_t notifyBit) {

    if (ui_mode == UI_MODE_LOGGING) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;

        xTaskNotifyFromISR(
            mode_mgr_handle,
            notifyBit,
            eSetBits,
            &xHigherPriorityTaskWoken
        );

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

// ======================================================
// Name: isr_up
// Description: ISR for UP button; performs debounce and
//              notifies mode manager.
// ======================================================
void IRAM_ATTR isr_up() {
    if (debounce(BTN_UP_ID, BTN_UP)) {
        if (ui_mode == UI_MODE_LOGGING) {
            notify_mode_manager(UI_NOTIFY_UP);
        }
    }
}

// ======================================================
// Name: isr_down
// Description: ISR for DOWN button; performs debounce and
//              notifies mode manager.
// ======================================================
void IRAM_ATTR isr_down() {
    if (debounce(BTN_DOWN_ID, BTN_DOWN)) {
        if (ui_mode == UI_MODE_LOGGING) {
            notify_mode_manager(UI_NOTIFY_DOWN);
        }
    }
}

// ======================================================
// Name: isr_enter
// Description: ISR for ENTER button; performs debounce and
//              notifies mode manager.
// ======================================================
void IRAM_ATTR isr_enter() {
    if (debounce(BTN_ENTER_ID, BTN_ENTER)) {
        if (ui_mode == UI_MODE_LOGGING) {
            notify_mode_manager(UI_NOTIFY_ENTER);
        }
    }
}

// ======================================================
// Name: mode_mgr_task
// Description: FreeRTOS task that manages UI transitions,
//              runs the configuration wizard, and applies
//              updated settings.
// ======================================================
void mode_mgr_task(void *arg) {

    uint32_t notifyValue = 0;

    while (true) {

        xTaskNotifyWait(
            0,
            ULONG_MAX,
            &notifyValue,
            portMAX_DELAY
        );

        if (ui_mode == UI_MODE_LOGGING) {

            Serial.println("UI MODE CHANGING!! LFG!!");

            ui_mode = UI_MODE_CONFIG;

            vTaskSuspend(regularDisplayTaskHandle);
            vTaskSuspend(read_sensors_handle);
            vTaskSuspend(displayNowTaskHandle);

            vTaskDelay(100 / portTICK_PERIOD_MS);

            run_config_wizard();

            ui_mode = UI_MODE_LOGGING;

            protocol_apply_schedule(global_config);

            vTaskDelay(100 / portTICK_PERIOD_MS);
            vTaskResume(regularDisplayTaskHandle);
            vTaskResume(read_sensors_handle);
            vTaskResume(displayNowTaskHandle);

            attach_buttons();
        }
    }
}

// ======================================================
// Name: debounce
// Description: Performs debounce filtering for button
//              presses using per-button timestamps.
// ======================================================
bool debounce(button_id_t id, int gpioPin) {

    unsigned long now = millis();
    volatile unsigned long *lastTime;

    switch (id) {
        case BTN_UP_ID:    lastTime = &lastDebounceUp;    break;
        case BTN_DOWN_ID:  lastTime = &lastDebounceDown;  break;
        case BTN_ENTER_ID: lastTime = &lastDebounceEnter; break;
    }

    if ((now - *lastTime) > DEBOUNCE_MS &&
        digitalRead(gpioPin) == LOW)
    {
        *lastTime = now;
        return true;
    }

    return false;
}

// ======================================================
// Name: attach_buttons
// Description: Attaches interrupts for UP, DOWN, and
//              ENTER buttons.
// ======================================================
void attach_buttons() {
    attachInterrupt(BTN_UP,    isr_up,    FALLING);
    attachInterrupt(BTN_DOWN,  isr_down,  FALLING);
    attachInterrupt(BTN_ENTER, isr_enter, FALLING);
}

// ======================================================
// Name: detach_buttons
// Description: Detaches interrupts for UP, DOWN, and
//              ENTER buttons.
// ======================================================
void detach_buttons() {
    detachInterrupt(BTN_UP);
    detachInterrupt(BTN_DOWN);
    detachInterrupt(BTN_ENTER);
}

// ======================================================
// Name: wait_for_button
// Description: Blocks until a button press is detected,
//              including press and release debounce.
// ======================================================
button_id_t wait_for_button() {
    const int debounce_ms = 40;
    const int release_ms  = 40;

    while (true) {

        if (digitalRead(BTN_UP) == LOW) {
            delay(debounce_ms);
            if (digitalRead(BTN_UP) == LOW) {
                while (digitalRead(BTN_UP) == LOW) delay(release_ms);
                return BTN_UP_ID;
            }
        }

        if (digitalRead(BTN_DOWN) == LOW) {
            delay(debounce_ms);
            if (digitalRead(BTN_DOWN) == LOW) {
                while (digitalRead(BTN_DOWN) == LOW) delay(release_ms);
                return BTN_DOWN_ID;
            }
        }

        if (digitalRead(BTN_ENTER) == LOW) {
            delay(debounce_ms);
            if (digitalRead(BTN_ENTER) == LOW) {
                while (digitalRead(BTN_ENTER) == LOW) delay(release_ms);
                return BTN_ENTER_ID;
            }
        }

        delay(10);
    }
}

// ======================================================
// Name: prompt_confirm
// Description: Displays a yes/no confirmation prompt and
//              returns true for YES.
// ======================================================
bool prompt_confirm(const char *message) {
    lcd.clear();
    lcd.print(message);
    lcd.setCursor(0, 1);
    lcd.print("UP=yes DOWN=no");

    button_id_t b = wait_for_button();
    return (b == BTN_UP_ID || b == BTN_ENTER_ID);
}

// ======================================================
// Name: run_config_wizard
// Description: Runs the interactive configuration wizard
//              for watering mode, thresholds, schedule,
//              temperature limits, and optional manual
//              watering.
// ======================================================
void run_config_wizard() {

    detach_buttons();

    if (!prompt_confirm("Enter config?")) {
        return;
    }

    config_msg_t temp = global_config;

    temp.mode = prompt_for_watering_mode(global_config.mode);

    if (temp.mode == MODE_THRESHOLD) {
        temp.threshold_pct =
            prompt_for_int("Threshold %:", global_config.threshold_pct, 0, 100);
    } else {
        temp.schedule_mins =
            prompt_for_int("Water mins:", global_config.schedule_mins, 0, 180);
    }

    temp.temp_min =
        prompt_for_int("Temp min:", global_config.temp_min, 0, 50);

    temp.temp_max =
        prompt_for_int("Temp max:", global_config.temp_max, temp.temp_min + 1, 50);

    if (prompt_confirm("Add water?")) {
        run_manual_water_prompt();
    }

    if (prompt_confirm("Apply settings?")) {
        global_config = temp;
    }
}

// ======================================================
// Name: run_manual_water_prompt
// Description: Runs interactive manual watering sequence
//              using START/STOP notifications.
// ======================================================
void run_manual_water_prompt() {

    lcd.clear();
    lcd.print("Press to START");
    lcd.setCursor(0,1);
    lcd.print("ENTER to begin");

    wait_for_button();

    xTaskNotify(manualWaterTaskHandle, 1, eSetValueWithoutOverwrite);

    lcd.clear();
    lcd.print("Dispensing...");
    lcd.setCursor(0,1);
    lcd.print("Press to STOP");

    wait_for_button();

    xTaskNotify(manualWaterTaskHandle, 2, eSetValueWithoutOverwrite);

    lcd.clear();
    lcd.print("Done!");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}

// ======================================================
// Name: prompt_for_int
// Description: Prompts user for an integer value within
//              a specified range.
// ======================================================
int prompt_for_int(const char *prompt, int initialValue, int minVal, int maxVal) {
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
            return value;
        }

        lcd.setCursor(0, 1);
        lcd.print("        ");
        lcd.setCursor(0, 1);
        lcd.print(value);
    }
}

// ======================================================
// Name: prompt_for_watering_mode
// Description: Prompts user to select watering mode
//              (THRESHOLD or SCHEDULE).
// ======================================================
control_mode_t prompt_for_watering_mode(control_mode_t initialMode) {
    control_mode_t mode = initialMode;

    lcd.clear();
    lcd.print("Watering mode:");
    lcd.setCursor(0, 1);
    lcd.print(mode == MODE_THRESHOLD ? "THRESHOLD" : "SCHEDULE");

    while (true) {
        button_id_t b = wait_for_button();

        if (b == BTN_UP_ID || b == BTN_DOWN_ID) {
            mode = (mode == MODE_THRESHOLD) ? MODE_SCHEDULE : MODE_THRESHOLD;

            lcd.setCursor(0, 1);
            lcd.print("          ");
            lcd.setCursor(0, 1);
            lcd.print(mode == MODE_THRESHOLD ? "THRESHOLD" : "SCHEDULE");
        }
        else if (b == BTN_ENTER_ID) {
            return mode;
        }
    }
}

#include "src/globals.h"
#include "src/protocol.h"
#include <Arduino.h>

// ======================================================
//  Binary packet buffer (ISR fills this)
// ======================================================

static uint8_t rx_buf[sizeof(config_msg_t)];
static uint8_t rx_idx = 0;

volatile unsigned long lastDebounceUp   = 0;
volatile unsigned long lastDebounceDown = 0;
volatile unsigned long lastDebounceEnter= 0;


// ======================================================
//  Read one byte from USART (Serial)
// ======================================================

uint8_t read_cmd_byte_from_hw() {
    return Serial.read();   // ISR-safe: interrupt fires only when data is available
}


// ======================================================
//  ISR: accumulate bytes until full config_msg_t received,
//       then queue the struct and reset buffer.
// ======================================================

void IRAM_ATTR cmd_isr() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // Read one byte from UART
    uint8_t b = read_cmd_byte_from_hw();

    // Store into buffer
    rx_buf[rx_idx++] = b;

    // Full struct received?
    if (rx_idx == sizeof(config_msg_t)) {

        // Reset index for next packet
        rx_idx = 0;

        // Convert raw bytes into a config_msg_t
        config_msg_t cfg;
        memcpy(&cfg, rx_buf, sizeof(cfg));

        // Push full struct to control loop
        xQueueSendFromISR(config_queue, &cfg, &xHigherPriorityTaskWoken);
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void command_task(void *pvParameters) {
    config_msg_t incoming;

    while (true) {
        if (xQueueReceive(config_queue, &incoming, portMAX_DELAY)) {

            memcpy((void*)&global_config, (void*)&incoming, sizeof(config_msg_t));

            protocol_apply_schedule(incoming);
            protocol_apply_threshold(incoming);

            // Later:
            // protocol_apply_mode(incoming);
            // protocol_apply_quantity(incoming);

                 vTaskDelay(GLOBAL_TICK);
        }
    }
}


static int last_schedule_hours = -1;   // only reconfigure when changed
static esp_timer_handle_t schedule_timer;

void protocol_apply_schedule(const config_msg_t &cfg) {

    // Only act if schedule changed
    if (cfg.schedule_hours != last_schedule_hours) {

        // Stop old timer
        esp_timer_stop(schedule_timer);

        // Start new timer if > 0
        if (cfg.schedule_hours > 0) {
            uint64_t interval_us =
                (uint64_t)cfg.schedule_hours * 3600ULL * 1000000ULL;

            esp_timer_start_periodic(schedule_timer, interval_us);
        }

        last_schedule_hours = cfg.schedule_hours;
    }
}

static int last_threshold_pct = -1;

void protocol_apply_threshold(const config_msg_t &cfg) {

    if (cfg.threshold_pct != last_threshold_pct) {

        // Update internal threshold state
        last_threshold_pct = cfg.threshold_pct;

        // (Optional) log or notify control loop
        // (Optional) reset hysteresis state
    }
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

    for (;;) {

        // Wait for ISR notification
        xTaskNotifyWait(
            0,
            ULONG_MAX,
            &notifyValue,
            portMAX_DELAY
        );

        // Only care about mode switching
        if (ui_mode == UI_MODE_LOGGING) {
            Serial.println("UI MODE CHANGING!! LFG!!");
            // Any button → switch to CONFIG mode
            ui_mode = UI_MODE_CONFIG;

            // Suspend logging-related tasks
            vTaskSuspend(regularDisplayTaskHandle);
            vTaskSuspend(read_sensors_handle);
            vTaskSuspend(displayNowTaskHandle);

            // Resume config-related tasks
            vTaskResume(mode_mgr_handle);

            continue;
        }

        // Later: handle switching back to LOGGING
        // (user_config_task will notify us when done)
        if (ui_mode == UI_MODE_CONFIG) {

            if (notifyValue & UI_NOTIFY_ENTER) {
                // ENTER means "done with config"
                ui_mode = UI_MODE_LOGGING
;

                // Resume logging tasks
                vTaskResume(regularDisplayTaskHandle);
                vTaskResume(read_sensors_handle);
                vTaskResume(displayNowTaskHandle);

                // Suspend config task
                vTaskSuspend(mode_mgr_handle);
            }

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








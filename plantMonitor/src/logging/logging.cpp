#include "src/logging.h"
#include "src/globals.h"
#include "src/actuators.h"

bool sound_alert_active = false;
bool temp_fault_active   = false;
bool humid_fault_active  = false;
bool light_fault_active  = false;
bool height_fault_active = false;
bool moist_fault_active  = false;
bool sound_fault_active  = false;   // separate from sound_alert_active


void logging_send_data(const plant_data_t &msg) {
    xQueueOverwrite(plant_data_queue, &msg);
}

void logging_print_data(const plant_data_t &msg) {
    Serial.print("ts=");
    Serial.print(msg.timestamp_ms);

    Serial.print(" moisture_pct=");
    Serial.print(msg.moisture_pct);
    Serial.print(" drying_rate=");
    Serial.print(msg.drying_rate_pct_per_hr);
    Serial.print(" moist_ok=");
    Serial.print(msg.moist_ok);

    Serial.print(" temp_c=");
    Serial.print(msg.temperature_c);
    Serial.print(" temp_ok=");
    Serial.print(msg.temp_ok);

    Serial.print(" humid_pct=");
    Serial.print(msg.humidity_pct);
    Serial.print(" humid_ok=");
    Serial.print(msg.humid_ok);

    Serial.print(" light_level=");
    Serial.print(msg.light_level);
    Serial.print(" light_ok=");
    Serial.print(msg.light_ok);

    Serial.print(" height_cm=");
    Serial.print(msg.height_cm);
    Serial.print(" growth_rate=");
    Serial.print(msg.growth_rate_cm_per_day);
    Serial.print(" height_ok=");
    Serial.print(msg.height_ok);

    Serial.print(" disturbance=");
    Serial.print(soundLevelToString(msg.disturbance_level));
    Serial.print(" sound_ok=");
    Serial.print(msg.sound_ok);

    Serial.print(" pump_ok=");
    Serial.print(msg.pump_ok);
    Serial.print(" vent_ok=");
    Serial.println(msg.vent_ok);
}

void logging_task(void *pvParameters)
{
    plant_data_t pkt;

    while (true) {

        if (xQueuePeek(plant_data_queue, &pkt, portMAX_DELAY)) {

            logging_print_data(pkt);
            bool hasRelevant = relevance_check(pkt, global_config);

            if (hasRelevant) {
                xTaskNotifyGive(displayNowTaskHandle);
            } else {
                if (!threshold_active) {
                    int dummy;
                    while (xQueueReceive(displayNowQueue, &dummy, 0)) {}
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(LOGGING_PERIOD));
    }
}

void regular_display_task(void *pv) {
    plant_data_t d;

    int screenIndex = 0;
    uint32_t lastSwitch = 0;

    const uint32_t SCREEN_INTERVAL_MS = 2000;

    while(true) {

        if (xQueuePeek(plant_data_queue, &d, portMAX_DELAY)) {

            if (millis() - lastSwitch > SCREEN_INTERVAL_MS) {
                screenIndex = (screenIndex + 1) % NUM_SCREENS;
                lastSwitch = millis();
            }

            if (!d.temp_ok   && screenIndex == SCREEN_TEMP)   { screenIndex = (screenIndex + 1) % NUM_SCREENS; continue; }
            if (!d.humid_ok  && screenIndex == SCREEN_HUMID)  { screenIndex = (screenIndex + 1) % NUM_SCREENS; continue; }
            if (!d.light_ok  && screenIndex == SCREEN_LIGHT)  { screenIndex = (screenIndex + 1) % NUM_SCREENS; continue; }
            if (!d.height_ok && screenIndex == SCREEN_HEIGHT) { screenIndex = (screenIndex + 1) % NUM_SCREENS; continue; }
            if (!d.moist_ok  && screenIndex == SCREEN_MOIST)  { screenIndex = (screenIndex + 1) % NUM_SCREENS; continue; }
            if (!d.sound_ok  && screenIndex == SCREEN_SOUND)  { screenIndex = (screenIndex + 1) % NUM_SCREENS; continue; }

            display_screen(screenIndex, d);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void display_screen(int idx, const plant_data_t &d) {
    lcd.clear();

    switch (idx) {

        case SCREEN_MOIST:
            lcd.setCursor(0,0);
            lcd.print("Moist: ");
            lcd.print(d.moisture_pct);
            lcd.print("%");
            break;

        case SCREEN_TEMP:
            lcd.setCursor(0,0);
            lcd.print("Temp: ");
            lcd.print(d.temperature_c);
            lcd.print("C");
            break;

        case SCREEN_LIGHT:
            lcd.setCursor(0,0);
            lcd.print("Light: ");
            lcd.print(d.light_level);
            break;

        case SCREEN_HUMID:
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Humidity:");
            lcd.setCursor(0,1);
            lcd.print(d.humidity_pct);
            break;

        case SCREEN_HEIGHT:
            lcd.setCursor(0,0);
            lcd.print("Height: ");
            lcd.print(d.height_cm);
            lcd.print("cm");
            break;

        case SCREEN_PUMP:
            lcd.setCursor(0,0);
            lcd.print("Pump OK: ");
            lcd.print(d.pump_ok ? "YES" : "NO");
            break;

        case SCREEN_VENT:
            lcd.setCursor(0,0);
            lcd.print("Vent OK: ");
            lcd.print(d.vent_ok ? "YES" : "NO");
            break;

        case SCREEN_SOUND:
            lcd.setCursor(0,0);
            lcd.print("Disturbance: ");
            lcd.print(soundLevelToString(d.disturbance_level));
            break;

        default:
            lcd.setCursor(0,0);
            lcd.print("Unknown");
            break;
    }
}

void display_alert_screen(int idx, const plant_data_t &d) {
    lcd.clear();

    lcd.setCursor(0,0);
    lcd.print("!!! ALERT !!!");

    lcd.setCursor(0,1);

    Serial.printf("alert screen received: %d\n", idx);

    switch (idx) {

        // Normal alert screens
        case SCREEN_TEMP:
            lcd.print("Temp: ");
            lcd.print(d.temperature_c);
            lcd.print("C");
            break;

        case SCREEN_MOIST:
            lcd.print("Moist: ");
            lcd.print(d.moisture_pct);
            lcd.print("%");
            break;

        case SCREEN_SOUND:
            lcd.print("Disturb: ");
            lcd.print(soundLevelToString(d.disturbance_level));
            break;

        // Fault alert screens
        case DISP_TOPIC_FAULT_TEMP:
            lcd.print("TEMP SENSOR FAULT");
            break;

        case DISP_TOPIC_FAULT_HUMID:
            lcd.print("HUMID SENSOR FAULT");
            break;

        case DISP_TOPIC_FAULT_LIGHT:
            lcd.print("LIGHT SENSOR FAULT");
            break;

        case DISP_TOPIC_FAULT_HEIGHT:
            lcd.print("HEIGHT SENSOR FAULT");
            break;

        case DISP_TOPIC_FAULT_MOIST:
            lcd.print("MOISTURE SENSOR FAULT");
            break;

        case DISP_TOPIC_FAULT_SOUND:
            lcd.print("SOUND SENSOR FAULT");
            break;

        default:
            lcd.print("Unknown");
            break;
    }
}


bool relevance_check(const plant_data_t &d,
                     const config_msg_t &cfg)
{
    bool queued = false;

    if (d.temperature_c > cfg.temp_max ){
        display_topic_t topic = DISP_TOPIC_TEMP;
        xQueueSend(displayNowQueue, &topic, 0);
        queued = true;
        vent_open();
    } else if( d.temperature_c < cfg.temp_min) {
        display_topic_t topic = DISP_TOPIC_TEMP;
        xQueueSend(displayNowQueue, &topic, 0);
        queued = true;
        vent_close();
    }

    if (d.disturbance_level == SOUND_HIGH) {
        if (!sound_alert_active) {
            display_topic_t topic = DISP_TOPIC_SOUND;
            xQueueSend(displayNowQueue, &topic, 0);
            queued = true;
        }
        sound_alert_active = true;
    } else {
        sound_alert_active = false;
    }


    if (cfg.mode == MODE_THRESHOLD) {

        const float low  = cfg.threshold_pct - 3.0f;
        const float high = cfg.threshold_pct + 3.0f;

        if (!threshold_active && d.moisture_pct < low) {

            display_topic_t topic2 = DISP_TOPIC_MOIST;
            xQueueSend(displayNowQueue, &topic2, 0);

            int dummy = 1;
            xQueueSend(threshold_queue, &dummy, 0);

            threshold_active = true;
            queued = true;
        }

        if (threshold_active && d.moisture_pct > high) {
            threshold_active = false;
        }
    }

    // --- Sensor Fault Detection (latched) ---
    if (!d.temp_ok) {
        if (!temp_fault_active) {
            display_topic_t t = DISP_TOPIC_FAULT_TEMP;
            xQueueSend(displayNowQueue, &t, 0);
            queued = true;
        }
        temp_fault_active = true;
    } else {
        temp_fault_active = false;
    }

    if (!d.humid_ok) {
        if (!humid_fault_active) {
            display_topic_t t = DISP_TOPIC_FAULT_HUMID;
            xQueueSend(displayNowQueue, &t, 0);
            queued = true;
        }
        humid_fault_active = true;
    } else {
        humid_fault_active = false;
    }

    if (!d.light_ok) {
        if (!light_fault_active) {
            display_topic_t t = DISP_TOPIC_FAULT_LIGHT;
            xQueueSend(displayNowQueue, &t, 0);
            queued = true;
        }
        light_fault_active = true;
    } else {
        light_fault_active = false;
    }

    if (!d.height_ok) {
        if (!height_fault_active) {
            display_topic_t t = DISP_TOPIC_FAULT_HEIGHT;
            xQueueSend(displayNowQueue, &t, 0);
            queued = true;
        }
        height_fault_active = true;
    } else {
        height_fault_active = false;
    }

    if (!d.moist_ok) {
        if (!moist_fault_active) {
            display_topic_t t = DISP_TOPIC_FAULT_MOIST;
            xQueueSend(displayNowQueue, &t, 0);
            queued = true;
        }
        moist_fault_active = true;
    } else {
        moist_fault_active = false;
    }

    if (!d.sound_ok) {
        if (!sound_fault_active) {
            display_topic_t t = DISP_TOPIC_FAULT_SOUND;
            xQueueSend(displayNowQueue, &t, 0);
            queued = true;
        }
        sound_fault_active = true;
    } else {
        sound_fault_active = false;
    }


    return queued;
}

void display_now_task(void *pv) {
    int idx;

    for (;;) {

        // Wait for logging_task to notify us
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // Pause regular rotation
        vTaskSuspend(regularDisplayTaskHandle);

        // Get first alert topic
        if (!xQueueReceive(displayNowQueue, &idx, 0)) {
            vTaskResume(regularDisplayTaskHandle);
            continue;
        }

        plant_data_t d;
        xQueuePeek(plant_data_queue, &d, 0);

        // Show first alert (topic directly)
        display_alert_screen(idx, d);
        vTaskDelay(pdMS_TO_TICKS(1500));

        // Persistent + multi-alert loop
        while (true) {

            // Always refresh latest plant data
            xQueuePeek(plant_data_queue, &d, 0);

            // Moisture threshold persistence
            if (idx == SCREEN_MOIST && threshold_active) {
                display_alert_screen(idx, d);
                vTaskDelay(pdMS_TO_TICKS(1000));
                continue;
            }

            // Drain additional queued alerts
            if (xQueueReceive(displayNowQueue, &idx, 0)) {
                display_alert_screen(idx, d);
                vTaskDelay(pdMS_TO_TICKS(1000));
                continue;
            }

            // No more alerts + moisture not active → exit
            break;
        }

        // Resume normal rotation
        vTaskResume(regularDisplayTaskHandle);
    }
}


int topic_to_index(display_topic_t t) {
    switch (t) {
        case DISP_TOPIC_TEMP:   return SCREEN_TEMP;
        case DISP_TOPIC_MOIST:  return SCREEN_MOIST;
        case DISP_TOPIC_SOUND:  return SCREEN_SOUND;
        case DISP_TOPIC_LIGHT:  return SCREEN_LIGHT;
        case DISP_TOPIC_HEIGHT: return SCREEN_HEIGHT;
        case DISP_TOPIC_PUMP:   return SCREEN_PUMP;
        case DISP_TOPIC_VENT:   return SCREEN_VENT;
        default:                return -1;
    }
}

void lcd_init() {
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("LCD Ready");
}

const char* soundLevelToString(sound_level_t lvl) {
    switch (lvl) {
        case SOUND_NOMINAL: return "NOMINAL";
        case SOUND_MEDIUM:  return "MEDIUM";
        case SOUND_HIGH:    return "HIGH";
        default:            return "UNKNOWN";
    }
}

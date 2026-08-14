#include "src/logging.h"
#include "src/globals.h"

void logging_send_data(const plant_data_t &msg) {
    xQueueOverwrite(plant_data_queue, &msg);

}

void logging_print_data(const plant_data_t &msg) {
    Serial.print("ts=");
    Serial.print(msg.timestamp_ms);

    // Moisture
    Serial.print(" moisture_pct=");
    Serial.print(msg.moisture_pct);
    Serial.print(" drying_rate=");
    Serial.print(msg.drying_rate_pct_per_hr);
    Serial.print(" moist_ok=");
    Serial.print(msg.moist_ok);

    // Temperature
    Serial.print(" temp_c=");
    Serial.print(msg.temperature_c);
    Serial.print(" temp_ok=");
    Serial.print(msg.temp_ok);

    // Humidity
    Serial.print(" humid_pct=");
    Serial.print(msg.humidity_pct);
    Serial.print(" humid_ok=");
    Serial.print(msg.humid_ok);

    // Light
    Serial.print(" light_level=");
    Serial.print(msg.light_level);
    Serial.print(" light_ok=");
    Serial.print(msg.light_ok);

    // Height
    Serial.print(" height_cm=");
    Serial.print(msg.height_cm);
    Serial.print(" growth_rate=");
    Serial.print(msg.growth_rate_cm_per_day);
    Serial.print(" height_ok=");
    Serial.print(msg.height_ok);

    // Sound
    Serial.print(" noise=");
    Serial.print(msg.noise_level);
    Serial.print(" disturb=");
    Serial.print(msg.disturbance_level);
    Serial.print(" sound_ok=");
    Serial.print(msg.sound_ok);

    // Actuator status
    Serial.print(" pump_ok=");
    Serial.print(msg.pump_ok);
    Serial.print(" vent_ok=");
    Serial.println(msg.vent_ok);
}

void logging_task(void *pvParameters)
{
    plant_data_t pkt;

    while (true) {
        // Block until a packet arrives
         if (xQueuePeek(plant_data_queue, &pkt, portMAX_DELAY)) {

            bool hasRelevant = relevance_check(pkt, global_config);

            if (hasRelevant) {
                // Notify display_now_task
                xTaskNotifyGive(displayNowTaskHandle);
            } else {
                int dummy;
                Serial.println("FLUSHED DISPLAY NOW QUEUE");
                while (xQueueReceive(displayNowQueue, &dummy, 0)) {}
            }

            // Normal logging
           // logging_print_data(pkt);
        }

        // Optional: throttle logging
        vTaskDelay(GLOBAL_TICK);
    }
}

void regular_display_task(void *pv) {
    plant_data_t d;

    int screenIndex = 0;
    uint32_t lastSwitch = 0;

    const uint32_t SCREEN_INTERVAL_MS = 2000;   // 2 seconds per screen
    const uint32_t UPDATE_RATE_MS     = 100;    // 10 Hz LCD update

    while(true) {

        // Always peek the most recent data
        if (xQueuePeek(plant_data_queue, &d, portMAX_DELAY)) {
            //Serial.println("display recieved plant data");
            // Time to switch to the next metric?
            if (millis() - lastSwitch > SCREEN_INTERVAL_MS) {
                screenIndex = (screenIndex + 1) % NUM_SCREENS;
                lastSwitch = millis();
            }

            // Render the selected screen
            display_screen(screenIndex, d);
        }

        vTaskDelay(GLOBAL_TICK);
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
            lcd.print("Noise: ");
            lcd.print(d.noise_level);
            break;

        default:
            lcd.setCursor(0,0);
            lcd.print("Unknown");
            break;
    }
}

bool relevance_check(const plant_data_t &d,
                     const config_msg_t &cfg)
{
    bool queued = false;
    display_topic_t topic;

    // Temperature out of range
    if (d.temperature_c > cfg.temp_max || d.temperature_c < cfg.temp_min) {

        Serial.println("TEMP OUT OF RANGE!!");
        topic = DISP_TOPIC_TEMP;
        xQueueSend(displayNowQueue, &topic, 0);

        // start the control loop to bring water threshold back up
  
        queued = true;
    }

    // Moisture below threshold (threshold mode only)
    if (cfg.mode == MODE_THRESHOLD &&
        d.moisture_pct < cfg.threshold_pct)
    {
        display_topic_t topic2;
        topic2 = DISP_TOPIC_MOIST;
        xQueueSend(displayNowQueue, &topic2, 0);


        //     Serial.println("WATER LEVEL DROPPED BELOW THRESHOLD!! LOOK OUT!!");
        // int dummy = 1;
        // xQueueSend(threshold_queue, &dummy, 0);
        queued = true;
    }

    return queued;
}

void display_now_task(void *pv) {
    int idx;

    for (;;) {

        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        Serial.println("RECEIVED A DISPLAY NOW EVENT!");

        vTaskSuspend(regularDisplayTaskHandle);

        if (!xQueueReceive(displayNowQueue, &idx, 0)) {  // this doesn't block
            Serial.println("DISPLAY_NOW: no topic available");
            vTaskResume(regularDisplayTaskHandle);
            continue;
        }

        plant_data_t d;
        xQueuePeek(plant_data_queue, &d, 0);


        display_screen(idx, d);
        vTaskDelay(pdMS_TO_TICKS(1500));

        while (xQueueReceive(displayNowQueue, &idx, 0)) {
            xQueuePeek(plant_data_queue, &d, 0);

            Serial.println("PRINTING PLANT DATA RECEIVED FROM DISPLAY NOW QUEUE");
            logging_print_data(d);
            display_screen(idx, d);
            vTaskDelay(pdMS_TO_TICKS(1500));
        }

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
    lcd.init();          // initialize LCD
    lcd.backlight();     // turn on backlight
    lcd.clear();         // clear screen

    lcd.setCursor(0,0);
    lcd.print("LCD Ready");
}


#include "src/logging.h"
#include "src/globals.h"

void logging_send_data(const plant_data_t &msg) {
    xQueueSend(plant_data_queue, &msg, 0);
}

void logging_print_data(const plant_data_t &msg) {
    Serial.print("ts=");
    Serial.print(msg.timestamp_ms);

    Serial.print(" moisture_raw=");
    Serial.print(msg.moisture_raw);

    Serial.print(" moisture_pct=");
    Serial.print(msg.moisture_pct);

    Serial.print(" temp_c=");
    Serial.print(msg.temperature_c);

    Serial.print(" light_raw=");
    Serial.print(msg.light_raw);

    Serial.print(" distance_mm=");
    Serial.print(msg.distance_mm);

    Serial.print(" mode=");
    Serial.print(msg.mode);

    Serial.print(" threshold=");
    Serial.print(msg.threshold_pct);

    Serial.print(" schedule=");
    Serial.print(msg.schedule_hours);

    Serial.print(" pump=");
    Serial.print(msg.pump_active);

    Serial.print(" vent=");
    Serial.println(msg.vent_open);
}

void logging_task(void *pvParameters)
{
    plant_data_t pkt;

    while (true) {
        // Block until a packet arrives
        if (xQueueReceive(plant_data_queue, &pkt, portMAX_DELAY)) {
            logging_print_data(pkt);
        }

        // Optional: throttle logging
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}




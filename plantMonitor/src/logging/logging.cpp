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

void logging_send_data() {
    plant_data_t pkt;

    // Drain all available packets
    while (xQueueReceive(plant_data_queue, &pkt, 0)) {

        // For now: print
        logging_print_data(pkt);

        // Later: send over UART/BLE/WiFi
        // send_plant_data_binary(pkt);
    }
}



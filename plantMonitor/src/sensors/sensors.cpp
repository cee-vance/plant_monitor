#include "src/sensors.h"
#include "src/globals.h"

void read_sensors_task(void *pvParameters) {
    while (true) {
        SensorData raw = read_raw_sensors();
        DerivedMetrics derived = compute_metrics(raw);

        plant_data_t pkt = build_plant_packet(raw, derived);

        xQueueSend(plant_data_queue, &pkt, 0);

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}


plant_data_t build_plant_packet(
    const SensorData &raw,
    const DerivedMetrics &derived
) {
    plant_data_t pkt;

    pkt.timestamp_ms = millis();

    // Raw sensor values
    pkt.moisture_raw = (uint16_t)(raw.moist_pct * 10);   // example scaling
    pkt.moisture_pct = raw.moist_pct;
    pkt.temperature_c = raw.temp;
    pkt.light_raw     = (uint16_t)raw.light_lux;
    pkt.distance_mm   = (uint16_t)(raw.height_cm * 10);

    // Config values
    pkt.mode           = global_config.mode;
    pkt.threshold_pct  = global_config.threshold_pct;
    pkt.schedule_hours = global_config.schedule_hours;

    // Actuator states
    pkt.pump_active = false;
    pkt.vent_open   = false;

    return pkt;
}

SensorData read_raw_sensors() {
    SensorData d;

    // --- Static placeholder values ---
    d.temp        = 24.5f;   // °C
    d.humidity    = 55.0f;   // %RH
    d.light_lux   = 1200.0f; // lux
    d.sound_rms   = 0.02f;   // arbitrary
    d.sound_peak  = 0.10f;   // arbitrary
    d.moist_pct   = 42.0f;   // %
    d.height_cm   = 18.0f;   // cm

    // --- All sensors OK for now ---
    d.temp_ok     = true;
    d.light_ok    = true;
    d.sound_ok    = true;
    d.moist_ok    = true;
    d.height_ok   = true;

    return d;
}


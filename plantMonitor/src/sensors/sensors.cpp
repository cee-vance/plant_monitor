#include "src/sensors.h"
#include "src/globals.h"

void read_sensors_task(void *pvParameters)
{
    const TickType_t tick = pdMS_TO_TICKS(100); // 100ms sensor period

    while (true)
    {
        // 1) Read raw sensor values
        SensorData raw = read_raw_sensors();
        
        uint32_t now_ms = millis();
        // 2) Compute derived environmental metrics
        DerivedMetrics derived = compute_plant_metrics(raw, now_ms);

        // 3) Build outbound plant_data_t packet
        plant_data_t pkt = build_plant_packet(raw, derived);

        // 4) Queue packet for logging
        xQueueSend(plant_data_queue, &pkt, 0);

        // 5) Sleep until next sensor tick
        vTaskDelay(tick);
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

float compute_heat_index(float temp_c, float humidity_pct) {
    // Convert to Fahrenheit for NOAA formula
    float T = temp_c * 9.0f / 5.0f + 32.0f;
    float R = humidity_pct;

    float HI = -42.379f +
               2.04901523f * T +
               10.14333127f * R -
               0.22475541f * T * R -
               0.00683783f * T * T -
               0.05481717f * R * R +
               0.00122874f * T * T * R +
               0.00085282f * T * R * R -
               0.00000199f * T * T * R * R;

    // Convert back to Celsius
    return (HI - 32.0f) * 5.0f / 9.0f;
}

float compute_dew_point(float temp_c, float humidity_pct) {
    const float a = 17.62f;
    const float b = 243.12f;

    float gamma = log(humidity_pct / 100.0f) + (a * temp_c) / (b + temp_c);
    return (b * gamma) / (a - gamma);
}

float photoperiod_hours = 0.0f;

void update_photoperiod(float light_lux, uint32_t tick_ms) {
    const float DAY_THRESHOLD = 500.0f; // tune later

    if (light_lux > DAY_THRESHOLD) {
        photoperiod_hours += (tick_ms / 3600000.0f); // ms → hours
    }
}

float last_moist_pct = 0.0f;
uint32_t last_moist_ts = 0;

float compute_drying_rate(float moist_pct, uint32_t now_ms) {

    
    float dt = (now_ms - last_moist_ts) / 1000.0f; // seconds
    float rate = 0.0f;

    if (dt > 0.0f) {
        rate = (moist_pct - last_moist_pct) / dt; // % per second
    }

    last_moist_pct = moist_pct;
    last_moist_ts = now_ms;

    return rate;
}

float last_height_cm = 0.0f;
uint32_t last_height_ts = 0;

float compute_growth_rate(float height_cm, uint32_t now_ms) {
    float dt = (now_ms - last_height_ts) / 3600000.0f; // hours
    float rate = 0.0f;

    if (dt > 0.0f) {
        rate = (height_cm - last_height_cm) / dt; // cm per hour
    }

    last_height_cm = height_cm;
    last_height_ts = now_ms;

    return rate;
}

float compute_noise_level(float rms) {
    return rms * 100.0f; // normalize later
}

float compute_disturbance_level(float peak, float rms) {
    float ratio = peak / (rms + 0.0001f);
    return ratio; // >3 = disturbance, tune later
}

DerivedMetrics compute_plant_metrics(const SensorData &sd, uint32_t now_ms) {
    DerivedMetrics pm;

    // Temperature-based metrics
    pm.heat_index = compute_heat_index(sd.temp, sd.humidity);
    pm.dew_point  = compute_dew_point(sd.temp, sd.humidity);

    // Light-based metrics
    update_photoperiod(sd.light_lux, 100); // sensor tick = 100ms
    pm.photoperiod = photoperiod_hours;

    // Sound-based metrics
    pm.noise_level       = compute_noise_level(sd.sound_rms);
    pm.disturbance_level = compute_disturbance_level(sd.sound_peak, sd.sound_rms);

    // Moisture-based metrics
    pm.drying_rate = compute_drying_rate(sd.moist_pct, now_ms);

    // Height-based metrics
    pm.growth_rate = compute_growth_rate(sd.height_cm, now_ms);

    return pm;
}








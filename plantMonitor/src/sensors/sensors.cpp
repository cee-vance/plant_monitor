#include "src/sensors.h"
#include "src/globals.h"
#include <Wire.h>
#include <Adafruit_AM2320.h>

Adafruit_AM2320 am2320;

void sensors_init() {
    delay(200);

    Wire.begin(SDA, SCLK);
    delay(10);
    am2320.begin();

    Serial.println("AM2320 starting...");

    // Ultrasonic
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    digitalWrite(TRIG_PIN, LOW);
    Serial.println("Ultrasonic sensor initialized.");

    // Light
    pinMode(LIGHT_PIN, INPUT);
    Serial.println("Light sensor initialized.");

    // Sound
    pinMode(SOUND_PIN, INPUT);
    Serial.println("Sound sensor initialized");

    // Moisture
    pinMode(MOISTURE_PIN, INPUT);
    Serial.println("Moisture sensor initialized.");
}

void read_sensors_task(void *pvParameters)
{
    const TickType_t tick = pdMS_TO_TICKS(100);

    while (true)
    {
        SensorData raw = read_raw_sensors();
        uint32_t now_ms = millis();

        DerivedMetrics derived = compute_plant_metrics(raw, now_ms);

        print_derived_metrics(derived);

        plant_data_t pkt = build_plant_packet(raw, derived);

        xQueueSend(plant_data_queue, &pkt, 0);

        vTaskDelay(GLOBAL_TICK);
    }
}

plant_data_t build_plant_packet(
    const SensorData &raw,
    const DerivedMetrics &derived
) {
    plant_data_t pkt;

    pkt.timestamp_ms = millis();

    // Moisture
    pkt.moisture_pct = raw.moist_pct;
    pkt.drying_rate_pct_per_hr = derived.drying_rate;
    pkt.moist_ok = raw.moist_ok;

    // Temperature
    pkt.temperature_c = raw.temp;
    pkt.temp_ok = raw.temp_ok;

    // Humidity
    pkt.humidity_pct = raw.humidity;
    pkt.humid_ok = raw.temp_ok;   // AM2320 gives both; temp_ok covers humidity validity

    // Light
    pkt.light_level = raw.light_lux;
    pkt.light_ok = raw.light_ok;

    // Height
    pkt.height_cm = raw.height_cm;
    pkt.growth_rate_cm_per_day = derived.growth_rate;
    pkt.height_ok = raw.height_ok;

    // Sound
    pkt.noise_level = derived.noise_level;
    pkt.disturbance_level = derived.disturbance_level;
    pkt.sound_ok = raw.sound_ok;

    // Actuator status (placeholder until control loop sets them)
    pkt.pump_ok = true;
    pkt.vent_ok = true;

    return pkt;
}

SensorData read_raw_sensors() {
    SensorData d;

    // --- AM2320 temperature + humidity ---
    d.temp     = am2320.readTemperature();
    d.humidity = am2320.readHumidity();
    d.temp_ok  = !isnan(d.temp) && !isnan(d.humidity);

    Serial.printf("temp: %.2f hum: %.2f\n", d.temp, d.humidity);

    // --- Height (ultrasonic) ---
    float h = read_ultrasonic_cm();
    if (isnan(h) || h <= 0 || h > 400) {
        d.height_cm = 0.0f;
        d.height_ok = false;
    } else {
        d.height_cm = h;
        d.height_ok = true;
    }

    // --- Light ---
    float light_val;
    if (read_light_sensor(light_val)) {
        d.light_lux = light_val;
        d.light_ok  = true;
    } else {
        d.light_lux = 0.0f;
        d.light_ok  = false;
    }

    // --- Sound ---
    float s;
    if (read_sound_sensor(s)) {
        d.sound_peak = s;
        d.sound_rms  = s;   // placeholder until RMS implemented
        d.sound_ok   = true;
    } else {
        d.sound_peak = 0.0f;
        d.sound_rms  = 0.0f;
        d.sound_ok   = false;
    }

    // --- Moisture ---
    float w;
    if (read_moisture_sensor(w)) {
        d.moist_pct = w;
        d.moist_ok  = true;
    } else {
        d.moist_pct = 0.0f;
        d.moist_ok  = false;
    }

    return d;
}

// --- Derived metric functions unchanged ---
float compute_heat_index(float temp_c, float humidity_pct) {
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
    const float DAY_THRESHOLD = 500.0f;

    if (light_lux > DAY_THRESHOLD) {
        photoperiod_hours += (tick_ms / 3600000.0f);
    }
}

float get_photoperiod_hours(void) {
    return photoperiod_hours;
}

float last_moist_pct = 0.0f;
uint32_t last_moist_ts = 0;

float compute_drying_rate(float moist_pct, uint32_t now_ms) {
    float dt = (now_ms - last_moist_ts) / 3600000.0f; // hours
    float rate = 0.0f;

    if (dt > 0.0f) {
        rate = (moist_pct - last_moist_pct) / dt;
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
        rate = (height_cm - last_height_cm) / dt;
    }

    last_height_cm = height_cm;
    last_height_ts = now_ms;

    return rate;
}

float compute_noise_level(float rms) {
    return rms * 100.0f;
}

float compute_disturbance_level(float peak, float rms) {
    float ratio = peak / (rms + 0.0001f);
    return ratio;
}

DerivedMetrics compute_plant_metrics(const SensorData &sd, uint32_t now_ms) {
    DerivedMetrics pm;

    pm.heat_index = compute_heat_index(sd.temp, sd.humidity);
    pm.dew_point  = compute_dew_point(sd.temp, sd.humidity);

    update_photoperiod(sd.light_lux, 100);
    pm.photoperiod = photoperiod_hours;

    pm.noise_level       = compute_noise_level(sd.sound_rms);
    pm.disturbance_level = compute_disturbance_level(sd.sound_peak, sd.sound_rms);

    pm.drying_rate = compute_drying_rate(sd.moist_pct, now_ms);
    pm.growth_rate = compute_growth_rate(sd.height_cm, now_ms);

    return pm;
}

float read_ultrasonic_cm() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH, 30000);

    if (duration == 0) {
        return NAN;
    }

    return duration * 0.034f / 2.0f;
}

bool read_light_sensor(float &lux_out) {
    int raw = analogRead(LIGHT_PIN);

    if (raw < 0 || raw > 4095) {
        return false;
    }

    lux_out = (float)raw;
    return true;
}

bool read_sound_sensor(float &sound_out) {
    int raw = analogRead(SOUND_PIN);

    if (raw < 0 || raw > 4095) {
        return false;
    }

    sound_out = (float)raw;
    return true;
}

void print_derived_metrics(const DerivedMetrics &dm) {
    Serial.println("=== Derived Metrics ===");

    Serial.print("Heat Index: ");
    Serial.println(dm.heat_index);

    Serial.print("Dew Point: ");
    Serial.println(dm.dew_point);

    Serial.print("Photoperiod (hrs): ");
    Serial.println(dm.photoperiod);

    Serial.print("Noise Level: ");
    Serial.println(dm.noise_level);

    Serial.print("Disturbance Level: ");
    Serial.println(dm.disturbance_level);

    Serial.print("Drying Rate (%/hr): ");
    Serial.println(dm.drying_rate);

    Serial.print("Growth Rate (cm/day): ");
    Serial.println(dm.growth_rate);

    Serial.println("=======================");
}

bool read_moisture_sensor(float &moist_pct) {
    int raw = analogRead(MOISTURE_PIN);

    if (raw < 0 || raw > 4095) {
        return false;
    }

    const float dry_raw = 3500.0f;
    const float wet_raw = 1200.0f;

    float pct = (raw - dry_raw) / (wet_raw - dry_raw);
    pct = constrain(pct, 0.0f, 1.0f);

    moist_pct = pct * 100.0f;
    return true;
}

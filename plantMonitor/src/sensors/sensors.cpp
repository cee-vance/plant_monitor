// ======================================================
//  Includes
// ======================================================
#include "src/sensors.h"
#include "src/globals.h"
#include <Wire.h>
#include <Adafruit_AM2320.h>

// ======================================================
//  AM2320 Sensor Object
// ======================================================
Adafruit_AM2320 am2320;

// ======================================================
//  Internal State (Photoperiod, Drying, Growth, Sound)
// ======================================================
namespace {
    float photoperiod_hours = 0.0f;

    float last_moist_pct = 0.0f;
    uint32_t last_moist_ts = 0;

    float last_height_cm = 0.0f;
    uint32_t last_height_ts = 0;

    // Sound processing state
    int buf[SOUND_WINDOW];
    int idx = 0;
    bool filled = false;

    long baseline = -1;
    long lastEnergy = 0;
}

// ======================================================
// Name: sensors_init
// Description: Initializes all sensor hardware including
//              I2C, ultrasonic, light, sound, and moisture.
// ======================================================
void sensors_init() {
    delay(200);

    Wire.begin(SDA_PIN, SCLK_PIN);
    delay(10);
    am2320.begin();

    Serial.println("AM2320 starting...");

    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    digitalWrite(TRIG_PIN, LOW);

    pinMode(LIGHT_PIN, INPUT);
    pinMode(SOUND_PIN, INPUT);
    pinMode(MOISTURE_PIN, INPUT);

    Serial.println("Sensors initialized.");
}

// ======================================================
// Name: read_sensors_task
// Description: FreeRTOS task that reads raw sensors,
//              computes derived metrics, and sends plant
//              data packets to the logging system.
// ======================================================
void read_sensors_task(void *pvParameters) {
    Serial.println("Sensor task started");

    while (true) {
        SensorData raw = read_raw_sensors();
        uint32_t now_ms = millis();

        xQueueOverwrite(raw_queue, &raw);

        DerivedMetrics derived = compute_plant_metrics(raw, now_ms);
        plant_data_t pkt = build_plant_packet(raw, derived);

        logging_send_data(pkt);

        vTaskDelay(pdMS_TO_TICKS(SENSOR_PERIOD));
    }
}

// ======================================================
// Name: build_plant_packet
// Description: Combines raw + derived sensor values into
//              a unified plant_data_t packet.
// ======================================================
plant_data_t build_plant_packet(
    const SensorData &raw,
    const DerivedMetrics &derived
) {
    plant_data_t pkt;

    pkt.timestamp_ms = millis();

    pkt.moisture_pct            = raw.moist_pct;
    pkt.drying_rate_pct_per_hr  = derived.drying_rate;
    pkt.moist_ok                = raw.moist_ok;

    pkt.temperature_c           = raw.temp;
    pkt.temp_ok                 = raw.temp_ok;

    pkt.humidity_pct            = raw.humidity;
    pkt.humid_ok                = raw.temp_ok;

    pkt.light_level             = raw.light_lux;
    pkt.light_ok                = raw.light_ok;

    pkt.height_cm               = raw.height_cm;
    pkt.growth_rate_cm_per_day  = derived.growth_rate;
    pkt.height_ok               = raw.height_ok;

    sound_level_t lvl;
    if (xQueuePeek(sound_queue, &lvl, 0) == pdTRUE)
        pkt.disturbance_level = lvl;
    else
        pkt.disturbance_level = SOUND_NOMINAL;

    pkt.sound_ok = raw.sound_ok;

    pkt.pump_ok = true;
    pkt.vent_ok = true;

    return pkt;
}

// ======================================================
// Name: read_raw_sensors
// Description: Reads all physical sensors and returns a
//              SensorData struct with raw values.
// ======================================================
SensorData read_raw_sensors() {
    SensorData d;

    d.temp     = am2320.readTemperature();
    d.humidity = am2320.readHumidity();
    d.temp_ok  = !isnan(d.temp) && !isnan(d.humidity);

    float h = read_ultrasonic_cm();
    if (isnan(h) || h <= 0 || h > 400) {
        d.height_cm = 0.0f;
        d.height_ok = false;
    } else {
        d.height_cm = h;
        d.height_ok = true;
    }

    float light_val;
    if (read_light_sensor(light_val)) {
        d.light_lux = light_val;
        d.light_ok  = true;
    } else {
        d.light_lux = 0.0f;
        d.light_ok  = false;
    }

    float s;
    if (read_sound_sensor(s)) {
        d.sound_peak = s;
        d.sound_rms  = s;
        d.sound_ok   = true;
    } else {
        d.sound_peak = 0.0f;
        d.sound_rms  = 0.0f;
        d.sound_ok   = false;
    }

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

// ======================================================
// Name: compute_heat_index
// Description: Computes heat index from temp + humidity.
// ======================================================
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

// ======================================================
// Name: compute_dew_point
// Description: Computes dew point from temp + humidity.
// ======================================================
float compute_dew_point(float temp_c, float humidity_pct) {
    const float a = 17.62f;
    const float b = 243.12f;

    float gamma = log(humidity_pct / 100.0f) + (a * temp_c) / (b + temp_c);
    return (b * gamma) / (a - gamma);
}

// ======================================================
// Name: update_photoperiod
// Description: Accumulates photoperiod hours based on
//              light intensity and elapsed time.
// ======================================================
void update_photoperiod(float light_lux, uint32_t tick_ms) {
    const float DAY_THRESHOLD = 500.0f;

    if (light_lux > DAY_THRESHOLD) {
        photoperiod_hours += (tick_ms / 3600000.0f);
    }
}

// ======================================================
// Name: get_photoperiod_hours
// Description: Returns accumulated photoperiod hours.
// ======================================================
float get_photoperiod_hours() {
    return photoperiod_hours;
}

// ======================================================
// Name: compute_drying_rate
// Description: Computes drying rate (%/hr) from moisture
//              readings over time.
// ======================================================
float compute_drying_rate(float moist_pct, uint32_t now_ms) {
    float dt = (now_ms - last_moist_ts) / 3600000.0f;
    float rate = 0.0f;

    if (dt > 0.0f)
        rate = (moist_pct - last_moist_pct) / dt;

    last_moist_pct = moist_pct;
    last_moist_ts  = now_ms;

    return rate;
}

// ======================================================
// Name: compute_growth_rate
// Description: Computes growth rate (cm/day) from height
//              readings over time.
// ======================================================
float compute_growth_rate(float height_cm, uint32_t now_ms) {
    float dt = (now_ms - last_height_ts) / 3600000.0f;
    float rate = 0.0f;

    if (dt > 0.0f)
        rate = (height_cm - last_height_cm) / dt;

    last_height_cm = height_cm;
    last_height_ts = now_ms;

    return rate;
}

// ======================================================
// Name: compute_noise_level
// Description: Computes normalized noise level from RMS.
// ======================================================
float compute_noise_level(float rms) {
    return rms * 100.0f;
}

// ======================================================
// Name: compute_disturbance_level
// Description: Computes disturbance level from peak/RMS.
// ======================================================
float compute_disturbance_level(float peak, float rms) {
    return peak / (rms + 0.0001f);
}

// ======================================================
// Name: compute_plant_metrics
// Description: Computes all derived environmental metrics
//              from raw sensor data.
// ======================================================
DerivedMetrics compute_plant_metrics(const SensorData &sd, uint32_t now_ms) {
    DerivedMetrics pm;

    pm.heat_index        = compute_heat_index(sd.temp, sd.humidity);
    pm.dew_point         = compute_dew_point(sd.temp, sd.humidity);

    update_photoperiod(sd.light_lux, 100);
    pm.photoperiod       = photoperiod_hours;

    pm.noise_level       = compute_noise_level(sd.sound_rms);
    pm.disturbance_level = compute_disturbance_level(sd.sound_peak, sd.sound_rms);

    pm.drying_rate       = compute_drying_rate(sd.moist_pct, now_ms);
    pm.growth_rate       = compute_growth_rate(sd.height_cm, now_ms);

    return pm;
}

// ======================================================
// Name: read_ultrasonic_cm
// Description: Reads HC-SR04 ultrasonic sensor distance.
// ======================================================
float read_ultrasonic_cm() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH, 30000);

    if (duration == 0)
        return NAN;

    return duration * 0.034f / 2.0f;
}

// ======================================================
// Name: read_light_sensor
// Description: Reads photoresistor and outputs lux.
// ======================================================
bool read_light_sensor(float &lux_out) {
    int raw = analogRead(LIGHT_PIN);

    if (raw < 800 || raw > 3800)
        return false;

    lux_out = (float)raw;
    return true;
}

// ======================================================
// Name: read_sound_sensor
// Description: Reads sound sensor and outputs amplitude.
// ======================================================
bool read_sound_sensor(float &sound_out) {
    int raw = analogRead(SOUND_PIN);

    if (raw < 0 || raw > 4095)
        return false;

    sound_out = (float)raw;
    return true;
}

// ======================================================
// Name: print_derived_metrics
// Description: Prints derived metrics for debugging.
// ======================================================
void print_derived_metrics(const DerivedMetrics &dm) {
    Serial.println("=== Derived Metrics ===");
    Serial.print("Heat Index: ");        Serial.println(dm.heat_index);
    Serial.print("Dew Point: ");         Serial.println(dm.dew_point);
    Serial.print("Photoperiod (hrs): "); Serial.println(dm.photoperiod);
    Serial.print("Noise Level: ");       Serial.println(dm.noise_level);
    Serial.print("Disturbance Level: "); Serial.println(dm.disturbance_level);
    Serial.print("Drying Rate (%/hr): ");Serial.println(dm.drying_rate);
    Serial.print("Growth Rate (cm/day): ");Serial.println(dm.growth_rate);
    Serial.println("=======================");
}

// ======================================================
// Name: read_moisture_sensor
// Description: Reads soil moisture sensor and outputs %.
// ======================================================
bool read_moisture_sensor(float &moist_pct) {
    int raw = analogRead(MOISTURE_PIN);

    if (raw < 0 || raw > 4095)
        return false;

    const float dry_raw = 3500.0f;
    const float wet_raw = 1200.0f;

    float pct = (raw - wet_raw) / (dry_raw - wet_raw);
    pct = constrain(pct, 0.0f, 1.0f);

    moist_pct = pct * 100.0f;
    return true;
}

// ======================================================
// Name: sound_disturbance_task
// Description: FreeRTOS task that computes sound energy,
//              baseline, delta, and disturbance level.
// ======================================================
void sound_disturbance_task(void *pv) {
    Serial.println("[VIB] vibrationTask started");

    while (true) {
        int raw = analogRead(SOUND_PIN);
        buf[idx] = raw;
        idx = (idx + 1) % SOUND_WINDOW;

        bool windowFinished = (idx == 0);
        if (windowFinished) filled = true;

        if (filled && windowFinished) {
            long energy = 0;
            for (int i = 0; i < SOUND_WINDOW; i++) {
                long v = buf[i];
                energy += v * v;
            }

            if (baseline < 0) {
                baseline = energy;
                lastEnergy = energy;
            }

            baseline = (baseline * 19 + energy) / 20;

            long delta = energy - lastEnergy;
            lastEnergy = energy;

            sound_level_t level;
            if (delta < 17000)      level = SOUND_NOMINAL;
            else if (delta < 36000) level = SOUND_MEDIUM;
            else                    level = SOUND_HIGH;

            xQueueOverwrite(sound_queue, &level);

            if (level == SOUND_HIGH) {
                Serial.printf("[VIB] raw=%d energy=%ld base=%ld delta=%ld level=%d\n",
                              raw, energy, baseline, delta, level);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(DISTURBANCE_PERIOD));
    }
}

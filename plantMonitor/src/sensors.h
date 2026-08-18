#pragma once

// ======================================================
//  Includes
// ======================================================
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_AM2320.h>
#include "src/logging.h"

// ======================================================
//  Macros / Constants
// ======================================================

// I2C pins (AM2320)
constexpr int SDA_PIN  = 21;
constexpr int SCLK_PIN = 20;

// Ultrasonic
constexpr int TRIG_PIN = 7;
constexpr int ECHO_PIN = 15;

// Photoresistor
constexpr int LIGHT_PIN = 1;

// Sound
constexpr int SOUND_PIN     = 2;
constexpr int SOUND_WINDOW  = 64;   // samples in energy window

// Moisture
constexpr int MOISTURE_PIN = 19;

// ======================================================
//  Global Objects
// ======================================================
extern Adafruit_AM2320 am2320;

// ======================================================
//  Data Structures
// ======================================================

// Raw Sensor Values (aligned with plant_data_t)
struct SensorData {
    float temp;        // °C
    float humidity;    // %
    float light_lux;   // lux
    float sound_rms;   // RMS amplitude
    float sound_peak;  // peak amplitude
    float moist_pct;   // %
    float height_cm;   // cm

    bool temp_ok;
    bool light_ok;
    bool sound_ok;
    bool moist_ok;
    bool height_ok;
};

// Derived Environmental Metrics (aligned with plant_data_t)
struct DerivedMetrics {
    float heat_index;
    float dew_point;
    float photoperiod;

    float noise_level;
    float disturbance_level;

    float drying_rate;
    float growth_rate;
};

// ======================================================
//  Function Prototypes
// ======================================================

// ------------------------------------------------------
// Name: read_sensors_task
// Description: FreeRTOS task that reads all sensors and
//              updates plant_data_queue.
// ------------------------------------------------------
void read_sensors_task(void *pvParameters);

// ------------------------------------------------------
// Name: read_raw_sensors
// Description: Reads all physical sensors and returns a
//              SensorData struct with raw values.
// ------------------------------------------------------
SensorData read_raw_sensors();

// ------------------------------------------------------
// Name: compute_metrics
// Description: Computes derived environmental metrics
//              from raw sensor values.
// ------------------------------------------------------
DerivedMetrics compute_metrics(const SensorData &raw);

// ------------------------------------------------------
// Name: build_plant_packet
// Description: Combines raw + derived metrics into a
//              plant_data_t packet for logging/display.
// ------------------------------------------------------
plant_data_t build_plant_packet(
    const SensorData &raw,
    const DerivedMetrics &derived
);

// ------------------------------------------------------
// Name: compute_heat_index
// Description: Computes heat index from temp + humidity.
// ------------------------------------------------------
float compute_heat_index(float temp_c, float humidity_pct);

// ------------------------------------------------------
// Name: compute_dew_point
// Description: Computes dew point from temp + humidity.
// ------------------------------------------------------
float compute_dew_point(float temp_c, float humidity_pct);

// ------------------------------------------------------
// Name: update_photoperiod
// Description: Updates accumulated photoperiod based on
//              light level and elapsed time.
// ------------------------------------------------------
void update_photoperiod(float light_lux, uint32_t tick_ms);

// ------------------------------------------------------
// Name: get_photoperiod_hours
// Description: Returns total photoperiod in hours.
// ------------------------------------------------------
float get_photoperiod_hours();

// ------------------------------------------------------
// Name: compute_drying_rate
// Description: Computes drying rate (%/hr) from moisture
//              readings over time.
// ------------------------------------------------------
float compute_drying_rate(float moist_pct, uint32_t now_ms);

// ------------------------------------------------------
// Name: compute_growth_rate
// Description: Computes growth rate (cm/day) from height
//              readings over time.
// ------------------------------------------------------
float compute_growth_rate(float height_cm, uint32_t now_ms);

// ------------------------------------------------------
// Name: compute_noise_level
// Description: Computes normalized noise level from RMS.
// ------------------------------------------------------
float compute_noise_level(float rms);

// ------------------------------------------------------
// Name: compute_disturbance_level
// Description: Computes disturbance level from peak + RMS.
// ------------------------------------------------------
float compute_disturbance_level(float peak, float rms);

// ------------------------------------------------------
// Name: print_derived_metrics
// Description: Prints derived metrics for debugging.
// ------------------------------------------------------
void print_derived_metrics(const DerivedMetrics &dm);

// ------------------------------------------------------
// Name: compute_plant_metrics
// Description: Legacy wrapper for derived metric compute.
// ------------------------------------------------------
DerivedMetrics compute_plant_metrics(const SensorData &sd, uint32_t now_ms);

// ------------------------------------------------------
// Name: sensors_init
// Description: Initializes all sensor hardware.
// ------------------------------------------------------
void sensors_init();

// ------------------------------------------------------
// Name: read_ultrasonic_cm
// Description: Reads HC-SR04 ultrasonic sensor distance.
// ------------------------------------------------------
float read_ultrasonic_cm();

// ------------------------------------------------------
// Name: read_light_sensor
// Description: Reads photoresistor and outputs lux.
// ------------------------------------------------------
bool read_light_sensor(float &lux_out);

// ------------------------------------------------------
// Name: read_sound_sensor
// Description: Reads sound sensor and outputs RMS/peak.
// ------------------------------------------------------
bool read_sound_sensor(float &sound_out);

// ------------------------------------------------------
// Name: sound_disturbance_task
// Description: FreeRTOS task that monitors sound levels
//              and triggers disturbance alerts.
// ------------------------------------------------------
void sound_disturbance_task(void *pv);

// ------------------------------------------------------
// Name: read_moisture_sensor
// Description: Reads soil moisture sensor and outputs %.
// ------------------------------------------------------
bool read_moisture_sensor(float &moist_pct);

#pragma once
#include <Arduino.h>
#include "src/logging.h"
#include <Wire.h>
#include <Adafruit_AM2320.h>

// ======================================================
//  Sensor Task
// ======================================================
void read_sensors_task(void *pvParameters);

// ======================================================
//  Raw Sensor Values
// ======================================================
struct SensorData {
    float temp;
    float humidity;
    float light_lux;
    float sound_rms;
    float sound_peak;
    float moist_pct;
    float height_cm;

    bool temp_ok;
    bool light_ok;
    bool sound_ok;
    bool moist_ok;
    bool height_ok;
};

// ======================================================
//  Derived Environmental Metrics
// ======================================================
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
//  Sensor Layer API
// ======================================================

SensorData read_raw_sensors();
DerivedMetrics compute_metrics(const SensorData &raw);

plant_data_t build_plant_packet(
    const SensorData &raw,
    const DerivedMetrics &derived
);

float compute_heat_index(float temp_c, float humidity_pct);
float compute_dew_point(float temp_c, float humidity_pct);

void update_photoperiod(float light_lux, uint32_t tick_ms);
float get_photoperiod_hours(void);

float compute_drying_rate(float moist_pct, uint32_t now_ms);
float compute_growth_rate(float height_cm, uint32_t now_ms);

float compute_noise_level(float rms);
float compute_disturbance_level(float peak, float rms);

DerivedMetrics compute_plant_metrics(const SensorData &sd, uint32_t now_ms);

// ======================================================
//  AM2320 Sensor Object
// ======================================================
extern Adafruit_AM2320 am2320;

void sensors_init();

// ======================================================
//  HC-SR04
// ======================================================
#define TRIG_PIN 6
#define ECHO_PIN 7

float read_ultrasonic_cm() ;

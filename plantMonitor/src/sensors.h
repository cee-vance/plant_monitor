#pragma once
#include <Arduino.h>
#include "src/logging.h"
#include <Wire.h>
#include <Adafruit_AM2320.h>

#define SDA 21
#define SCLK 20





// ======================================================
//  Sensor Task
// ======================================================
void read_sensors_task(void *pvParameters);

// ======================================================
//  Raw Sensor Values (aligned with final plant_data_t)
// ======================================================
struct SensorData {
    float temp;         // °C
    float humidity;     // %
    float light_lux;    // raw lux or normalized
    float sound_rms;    // RMS amplitude
    float sound_peak;   // peak amplitude
    float moist_pct;    // %
    float height_cm;    // cm

    bool temp_ok;
    bool light_ok;
    bool sound_ok;
    bool moist_ok;
    bool height_ok;
};

// ======================================================
//  Derived Environmental Metrics (aligned with final plant_data_t)
// ======================================================
struct DerivedMetrics {
    float heat_index;
    float dew_point;
    float photoperiod;

    float noise_level;         // normalized or scaled
    float disturbance_level;   // quiet/moderate/loud

    float drying_rate;         // %/hr
    float growth_rate;         // cm/day
};

// ======================================================
//  Sensor Layer API
// ======================================================

// Reads raw sensors (implemented in sensors.cpp)
SensorData read_raw_sensors();

// Computes derived metrics (implemented in sensors.cpp)
DerivedMetrics compute_metrics(const SensorData &raw);

// Builds outbound plant_data_t packet (implemented in sensors.cpp)
plant_data_t build_plant_packet(
    const SensorData &raw,
    const DerivedMetrics &derived
);

// Individual metric helpers
float compute_heat_index(float temp_c, float humidity_pct);
float compute_dew_point(float temp_c, float humidity_pct);

void update_photoperiod(float light_lux, uint32_t tick_ms);
float get_photoperiod_hours(void);

float compute_drying_rate(float moist_pct, uint32_t now_ms);
float compute_growth_rate(float height_cm, uint32_t now_ms);

float compute_noise_level(float rms);
float compute_disturbance_level(float peak, float rms);

// debugging 
void print_derived_metrics(const DerivedMetrics &dm);

// Legacy name preserved for compatibility
DerivedMetrics compute_plant_metrics(const SensorData &sd, uint32_t now_ms);

// ======================================================
//  AM2320 Sensor Object
// ======================================================
extern Adafruit_AM2320 am2320;

void sensors_init();

// ======================================================
//  HC-SR04
// ======================================================
#define TRIG_PIN 7
#define ECHO_PIN 15

float read_ultrasonic_cm();

// ======================================================
//  Photoresistor
// ======================================================
#define LIGHT_PIN 1

bool read_light_sensor(float &lux_out);

// ======================================================
//  Sound
// ======================================================
#define SOUND_PIN 2

#define WINDOW     64        // samples in energy window

// --- STATE ---
static int buf[WINDOW];
static int idx = 0;
static bool filled = false;

static long baseline = -1;   // -1 means “uninitialized”
static long lastEnergy = 0;


bool read_sound_sensor(float &sound_out);
void sound_disturbance_task(void *pv);

// ======================================================
//  Moisture
// ======================================================
#define MOISTURE_PIN 19

bool read_moisture_sensor(float &moist_pct);

#pragma once
#include <Arduino.h>
#include "src/logging.h"

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
//  Sensor Layer API (Declarations Only)
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

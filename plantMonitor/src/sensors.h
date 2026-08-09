#pragma once
#include <Arduino.h>

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

struct DerivedMetrics {
    float heat_index;
    float dew_point;
    float photoperiod;
    float noise_level;
    float disturbance_level;
    float drying_rate;
    float growth_rate;
};



#pragma once
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "src/protocol.h"   // for control_mode_t
#include "src/globals.h"




// ======================================================
//  Unified Plant Data Packet (FINAL SENSOR-CENTRIC MODEL)
// ======================================================

struct plant_data_t {
    uint32_t timestamp_ms;

    // Moisture
    float moisture_pct;
    float drying_rate_pct_per_hr;
    bool  moist_ok;

    // Temperature
    float temperature_c;
    bool  temp_ok;

    // Humidity
    float humidity_pct;
    bool  humid_ok;

    // Light
    float light_level;
    bool  light_ok;

    // Height
    float height_cm;
    float growth_rate_cm_per_day;
    bool  height_ok;

    // Sound
    sound_level_t disturbance_level;
    bool  sound_ok;

    // Actuator status (fault detection only)
    bool pump_ok;
    bool vent_ok;
};

// ======================================================
//  Display Topics (unchanged)
// ======================================================

typedef enum {
    DISP_TOPIC_TEMP   = SCREEN_TEMP,    // 0
    DISP_TOPIC_MOIST  = SCREEN_MOIST,   // 1
    DISP_TOPIC_HUMID  = SCREEN_HUMID,   // 2
    DISP_TOPIC_LIGHT  = SCREEN_LIGHT,   // 3
    DISP_TOPIC_SOUND  = SCREEN_SOUND,   // 4
    DISP_TOPIC_HEIGHT = SCREEN_HEIGHT,  // 5
    DISP_TOPIC_PUMP   = SCREEN_PUMP,    // 6
    DISP_TOPIC_VENT   = SCREEN_VENT     // 7
} display_topic_t;




// Queue for outgoing plant data
extern QueueHandle_t plant_data_queue;

// ======================================================
//  Logging API (implemented in logging.cpp)
// ======================================================

// Push a plant_data_t packet into the queue
void logging_send_data(const plant_data_t &msg);

// Task for sending plant data
void logging_task(void *pvParameters);

// Print formatted plant data (serial, BLE, etc.)
void logging_print_data(const plant_data_t &msg);

// ======================================================
//  Display Plant Data tasks & helpers
// ======================================================

void regular_display_task(void *pv);

void display_now_task(void *pv);

void display_screen(int idx, const plant_data_t &d);

bool relevance_check(const plant_data_t &d,
                     const config_msg_t &cfg);

int topic_to_index(display_topic_t t);

void lcd_init() ;

const char* soundLevelToString(sound_level_t lvl);
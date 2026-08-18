#pragma once

// ======================================================
//  Includes
// ======================================================
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "src/protocol.h"
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
//  Display Topics
// ======================================================
typedef enum {
    DISP_TOPIC_TEMP   = SCREEN_TEMP,
    DISP_TOPIC_MOIST  = SCREEN_MOIST,
    DISP_TOPIC_SOUND  = SCREEN_SOUND,
    DISP_TOPIC_LIGHT  = SCREEN_LIGHT,
    DISP_TOPIC_HEIGHT = SCREEN_HEIGHT,
    DISP_TOPIC_PUMP   = SCREEN_PUMP,
    DISP_TOPIC_VENT   = SCREEN_VENT,

    // Fault topics
    DISP_TOPIC_FAULT_TEMP,
    DISP_TOPIC_FAULT_HUMID,
    DISP_TOPIC_FAULT_LIGHT,
    DISP_TOPIC_FAULT_HEIGHT,
    DISP_TOPIC_FAULT_MOIST,
    DISP_TOPIC_FAULT_SOUND
} display_topic_t;

// ======================================================
//  Queue Handle
// ======================================================
extern QueueHandle_t plant_data_queue;

// ======================================================
//  Logging API
// ======================================================

// ------------------------------------------------------
// Name: logging_send_data
// Description: Pushes a plant_data_t packet into the
//              plant_data_queue for downstream tasks.
// ------------------------------------------------------
void logging_send_data(const plant_data_t &msg);

// ------------------------------------------------------
// Name: logging_task
// Description: FreeRTOS task that receives plant data,
//              performs relevance checks, and triggers
//              display or alert tasks.
// ------------------------------------------------------
void logging_task(void *pvParameters);

// ------------------------------------------------------
// Name: logging_print_data
// Description: Prints formatted plant data to serial or
//              BLE for debugging and monitoring.
// ------------------------------------------------------
void logging_print_data(const plant_data_t &msg);

// ======================================================
//  Display Tasks & Helpers
// ======================================================

// ------------------------------------------------------
// Name: regular_display_task
// Description: Cycles through normal display screens in
//              a timed rotation.
// ------------------------------------------------------
void regular_display_task(void *pv);

// ------------------------------------------------------
// Name: display_now_task
// Description: Handles alert screens and overrides the
//              regular display rotation when necessary.
// ------------------------------------------------------
void display_now_task(void *pv);

// ------------------------------------------------------
// Name: display_screen
// Description: Renders a specific screen index using the
//              provided plant data.
// ------------------------------------------------------
void display_screen(int idx, const plant_data_t &d);

// ------------------------------------------------------
// Name: relevance_check
// Description: Determines whether plant data contains a
//              condition requiring immediate display.
// ------------------------------------------------------
bool relevance_check(const plant_data_t &d,
                     const config_msg_t &cfg);

// ------------------------------------------------------
// Name: topic_to_index
// Description: Converts a display_topic_t to a screen
//              index for the LCD display system.
// ------------------------------------------------------
int topic_to_index(display_topic_t t);

// ------------------------------------------------------
// Name: lcd_init
// Description: Initializes the LCD hardware and prepares
//              it for rendering screens.
// ------------------------------------------------------
void lcd_init();

// ------------------------------------------------------
// Name: soundLevelToString
// Description: Converts a sound_level_t enum to a human-
//              readable string.
// ------------------------------------------------------
const char* soundLevelToString(sound_level_t lvl);

// ------------------------------------------------------
// Name: display_alert_screen
// Description: Renders an alert screen for the given
//              topic index and plant data.
// ------------------------------------------------------
void display_alert_screen(int idx, const plant_data_t &d);

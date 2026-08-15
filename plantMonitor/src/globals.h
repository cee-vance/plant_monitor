
#ifndef GLOBALS_H
#define GLOBALS_H



#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <LiquidCrystal_I2C.h>

// data indexes for display
#define SCREEN_TEMP    0
#define SCREEN_MOIST   1
#define SCREEN_HUMID   2
#define SCREEN_LIGHT   3
#define SCREEN_SOUND   4
#define SCREEN_HEIGHT  5
#define SCREEN_PUMP    6
#define SCREEN_VENT    7

#define NUM_SCREENS    8

#define UI_NOTIFY_UP     (1 << 0)
#define UI_NOTIFY_DOWN   (1 << 1)
#define UI_NOTIFY_ENTER  (1 << 2)


#define GLOBAL_SCHED_ML  .5         // always disense half a mL on schedule

// GLOBAL task periods
#define DISTURBANCE_PERIOD 15   // approx. 64 Hz 
#define SENSOR_PERIOD       20  // 50 Hz

#define LOGGING_PERIOD      100 // 10 Hz



enum ui_mode_t {
    UI_MODE_LOGGING = 0,   // normal operation
    UI_MODE_CONFIG  = 1    // user config wizard active
};

extern volatile ui_mode_t ui_mode;



// Global config (shared across tasks)
struct config_msg_t;   // forward declaration
extern config_msg_t global_config;

// Queue for config updates
extern QueueHandle_t config_queue;

// Task handle for command_task
extern TaskHandle_t command_task_handle;

// Sensors task & queues
extern QueueHandle_t plant_data_queue;
extern TaskHandle_t logging_task_handle;

extern TaskHandle_t read_sensors_handle;

// Vent queue 
extern QueueHandle_t vent_queue;

extern TaskHandle_t control_loop_handle;


extern const TickType_t GLOBAL_TICK;


// --- Display Task Handles ---
extern TaskHandle_t regularDisplayTaskHandle;
extern TaskHandle_t displayNowTaskHandle;

// --- Queue for display topics (semantic events) ---
extern QueueHandle_t displayNowQueue;



extern QueueHandle_t schedule_queue;

extern LiquidCrystal_I2C lcd;

// state mgmt 
extern TaskHandle_t mode_mgr_handle;

// threshold

extern QueueHandle_t threshold_queue;


extern QueueHandle_t raw_queue;

extern TaskHandle_t manualWaterTaskHandle;

extern bool threshold_active;


extern QueueHandle_t sound_queue;

typedef enum {
    SOUND_NOMINAL = 0,
    SOUND_MEDIUM  = 1,
    SOUND_HIGH    = 2
} sound_level_t;


#endif








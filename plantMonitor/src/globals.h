
#pragma once

// ======================================================
//  Includes
// ======================================================
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <LiquidCrystal_I2C.h>

// ======================================================
//  Display Screen Indexes
// ======================================================
#define SCREEN_TEMP    0
#define SCREEN_MOIST   1
#define SCREEN_HUMID   2
#define SCREEN_LIGHT   3
#define SCREEN_SOUND   4
#define SCREEN_HEIGHT  5
#define SCREEN_PUMP    6
#define SCREEN_VENT    7

#define NUM_SCREENS    8

// ======================================================
//  UI Notifications
// ======================================================
#define UI_NOTIFY_UP     (1 << 0)
#define UI_NOTIFY_DOWN   (1 << 1)
#define UI_NOTIFY_ENTER  (1 << 2)

// ======================================================
//  Global Constants
// ======================================================
#define GLOBAL_SCHED_ML      0.5f   // dispense 0.5 mL on schedule

#define DISTURBANCE_PERIOD   15     // ~64 Hz
#define SENSOR_PERIOD        20     // 50 Hz
#define LOGGING_PERIOD       100    // 10 Hz

// ======================================================
//  UI Mode
// ======================================================
enum ui_mode_t {
    UI_MODE_LOGGING = 0,   // normal operation
    UI_MODE_CONFIG  = 1    // user config wizard active
};

extern volatile ui_mode_t ui_mode;

// ======================================================
//  Global Config
// ======================================================
struct config_msg_t;        // forward declaration
extern config_msg_t global_config;

// Queue for config updates
extern QueueHandle_t config_queue;

// ======================================================
//  Task Handles
// ======================================================
extern TaskHandle_t command_task_handle;

extern TaskHandle_t logging_task_handle;
extern TaskHandle_t read_sensors_handle;
extern TaskHandle_t control_loop_handle;

extern TaskHandle_t regularDisplayTaskHandle;
extern TaskHandle_t displayNowTaskHandle;

extern TaskHandle_t mode_mgr_handle;
extern TaskHandle_t manualWaterTaskHandle;

// ======================================================
//  Queues
// ======================================================
extern QueueHandle_t plant_data_queue;
extern QueueHandle_t vent_queue;
extern QueueHandle_t displayNowQueue;
extern QueueHandle_t schedule_queue;
extern QueueHandle_t threshold_queue;
extern QueueHandle_t raw_queue;
extern QueueHandle_t sound_queue;

// ======================================================
//  Timing
// ======================================================
extern const TickType_t GLOBAL_TICK;

// ======================================================
//  LCD Object
// ======================================================
extern LiquidCrystal_I2C lcd;

// ======================================================
//  Sound Level Enum
// ======================================================
typedef enum {
    SOUND_NOMINAL = 0,
    SOUND_MEDIUM  = 1,
    SOUND_HIGH    = 2
} sound_level_t;

// ======================================================
//  Threshold State
// ======================================================
extern bool threshold_active;

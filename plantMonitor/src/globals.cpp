// ======================================================
//  Includes
// ======================================================
#include "src/globals.h"
#include "src/protocol.h"

// ======================================================
//  Global Configuration
// ======================================================
// Name: global_config
// Description: Default configuration for control logic.
//              Values are overwritten by the config wizard.
config_msg_t global_config = {
    .mode           = MODE_THRESHOLD,
    .threshold_pct  = 5,
    .schedule_mins  = 5,
    .temp_min       = 20,
    .temp_max       = 33
};

// ======================================================
//  Global Tick Period
// ======================================================
// Name: GLOBAL_TICK
// Description: Base tick period used by timing logic.
const TickType_t GLOBAL_TICK = pdMS_TO_TICKS(1000);

// ======================================================
//  Global Queues
// ======================================================
QueueHandle_t config_queue      = nullptr;
QueueHandle_t plant_data_queue  = nullptr;
QueueHandle_t vent_queue        = nullptr;
QueueHandle_t displayNowQueue   = nullptr;
QueueHandle_t schedule_queue    = nullptr;
QueueHandle_t threshold_queue   = nullptr;
QueueHandle_t raw_queue         = nullptr;
QueueHandle_t sound_queue       = nullptr;

// ======================================================
//  Global Task Handles
// ======================================================
TaskHandle_t command_task_handle       = nullptr;
TaskHandle_t logging_task_handle       = nullptr;
TaskHandle_t read_sensors_task         = nullptr;
TaskHandle_t read_sensors_handle       = nullptr;
TaskHandle_t control_loop_handle       = nullptr;
TaskHandle_t regularDisplayTaskHandle  = nullptr;
TaskHandle_t displayNowTaskHandle      = nullptr;
TaskHandle_t mode_mgr_handle           = nullptr;
TaskHandle_t manualWaterTaskHandle     = nullptr;

// ======================================================
//  LCD Instance
// ======================================================
// Name: lcd
// Description: Global LCD object for display tasks.
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ======================================================
//  UI Mode State
// ======================================================
// Name: ui_mode
// Description: Tracks whether system is in LOGGING or CONFIG mode.
volatile ui_mode_t ui_mode = UI_MODE_LOGGING;

// ======================================================
//  Threshold Watering State
// ======================================================
// Name: threshold_active
// Description: Latches threshold watering state to prevent repeated triggers.
bool threshold_active = false;

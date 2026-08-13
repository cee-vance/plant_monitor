#include "src/globals.h"
#include "src/protocol.h"

config_msg_t global_config = {
    .mode = MODE_THRESHOLD,   // or MODE_SCHEDULE, doesn't matter yet
    .threshold_pct = 40,
    .schedule_hours = 0,

    .temp_min = 20,           // start opening at 20°C
    .temp_max = 33            // fully open at 28°C
};          // for debugging

const TickType_t GLOBAL_TICK = pdMS_TO_TICKS(1000);

QueueHandle_t config_queue = nullptr;

TaskHandle_t command_task_handle = nullptr;

QueueHandle_t plant_data_queue = nullptr;
TaskHandle_t logging_task_handle = nullptr;
TaskHandle_t read_sensors_task = nullptr;
TaskHandle_t read_sensors_handle = nullptr;

QueueHandle_t vent_queue = nullptr;

TaskHandle_t control_loop_handle = nullptr;


// --- Task Handles ---
TaskHandle_t regularDisplayTaskHandle = nullptr;
TaskHandle_t displayNowTaskHandle     = nullptr;

// --- Queue for display topics (semantic events) ---
QueueHandle_t displayNowQueue = nullptr;

LiquidCrystal_I2C lcd(0x27, 16, 2);   // your LCD address + size

TaskHandle_t mode_mgr_handle = nullptr;

volatile ui_mode_t ui_mode = UI_MODE_LOGGING;


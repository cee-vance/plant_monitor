#include "globals.h"

config_msg_t global_config = {
    .mode = MODE_THRESHOLD,   // or MODE_SCHEDULE, doesn't matter yet
    .threshold_pct = 40,
    .schedule_hours = 0,

    .temp_min = 20,           // start opening at 20°C
    .temp_max = 28            // fully open at 28°C
};          // for debugging

QueueHandle_t config_queue = nullptr;

TaskHandle_t command_task_handle = nullptr;

QueueHandle_t plant_data_queue = nullptr;
TaskHandle_t logging_task_handle = nullptr;
TaskHandle_t read_sensors_task = nullptr;
TaskHandle_t read_sensors_handle = nullptr;

QueueHandle_t vent_queue = nullptr;

TaskHandle_t control_loop_handle = nullptr;
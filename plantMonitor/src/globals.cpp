#include "globals.h"

volatile config_msg_t global_config = {};
QueueHandle_t config_queue = nullptr;

TaskHandle_t command_task_handle = nullptr;

QueueHandle_t plant_data_queue = nullptr;
TaskHandle_t logging_task_handle = nullptr;
TaskHandle_t read_sensors_task = nullptr;
TaskHandle_t read_sensors_handle = nullptr;
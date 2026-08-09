#include "globals.h"

volatile config_msg_t global_config = {};
QueueHandle_t config_queue = nullptr;

TaskHandle_t command_task_handle = nullptr;


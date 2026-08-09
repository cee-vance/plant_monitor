#pragma once
#include "src/protocol.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

// Global config (shared across tasks)
extern volatile config_msg_t global_config;

// Queue for config updates
extern QueueHandle_t config_queue;

// Task handle for command_task
extern TaskHandle_t command_task_handle;


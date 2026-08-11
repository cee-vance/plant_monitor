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

// Sensors task & queues
extern QueueHandle_t plant_data_queue;
extern TaskHandle_t logging_task_handle;

extern TaskHandle_t read_sensors_handle;


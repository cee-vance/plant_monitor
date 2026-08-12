#pragma once
#include "src/protocol.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"


// Global config (shared across tasks)
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

static const TickType_t GLOBAL_TICK = pdMS_TO_TICKS(1000);   // 1000ms




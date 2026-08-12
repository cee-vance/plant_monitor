#pragma once
#include "src/protocol.h"
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

#pragma once

extern const TickType_t GLOBAL_TICK;


// --- Display Task Handles ---
extern TaskHandle_t regularDisplayTaskHandle;
extern TaskHandle_t displayNowTaskHandle;

// --- Queue for display topics (semantic events) ---
extern QueueHandle_t displayNowQueue;




extern LiquidCrystal_I2C lcd;





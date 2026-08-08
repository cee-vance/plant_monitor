#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sensor_tasks.h"
#include "control_loop.h"

// Queue definitions
QueueHandle_t temp_queue       = NULL;
QueueHandle_t moisture_queue   = NULL;
QueueHandle_t light_queue      = NULL;
QueueHandle_t sound_queue      = NULL;
QueueHandle_t ultrasonic_queue = NULL;

// Task handles
TaskHandle_t temp_task_handle        = NULL;
TaskHandle_t moisture_task_handle    = NULL;
TaskHandle_t light_task_handle       = NULL;
TaskHandle_t sound_task_handle       = NULL;
TaskHandle_t ultrasonic_task_handle  = NULL;
TaskHandle_t control_task_handle     = NULL;

// Actuator queues
QueueHandle_t pump_queue      = NULL;
QueueHandle_t light_act_queue = NULL;
QueueHandle_t fan_queue       = NULL;

// Actuator task handles
TaskHandle_t pump_task_handle      = NULL;
TaskHandle_t light_act_task_handle = NULL;
TaskHandle_t fan_task_handle       = NULL;


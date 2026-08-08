#ifndef ACTUATOR_TASKS_H
#define ACTUATOR_TASKS_H

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Actuator command message
typedef struct {
    uint32_t timestamp;
    float value;       // e.g., duty cycle, on/off, intensity
} actuator_msg_t;

// Queues (extern)
extern QueueHandle_t pump_queue;
extern QueueHandle_t light_act_queue;
extern QueueHandle_t fan_queue;

// Task handles (extern)
extern TaskHandle_t pump_task_handle;
extern TaskHandle_t light_act_task_handle;
extern TaskHandle_t fan_task_handle;

// Task prototypes
void pump_task(void *pv);
void light_actuator_task(void *pv);
void fan_task(void *pv);

#endif

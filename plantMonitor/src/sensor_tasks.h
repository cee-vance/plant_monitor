// sensor_tasks.h
#ifndef SENSOR_TASKS_H
#define SENSOR_TASKS_H

#include <Arduino.h>
#include <FreeRTOS.h>
#include <queue.h>

// ========================= Shared Message Struct =========================
typedef struct {
    uint32_t timestamp;
    float value;
} sensor_msg_t;

// ========================= Queue Handles =========================
extern QueueHandle_t temp_queue;
extern QueueHandle_t moisture_queue;
extern QueueHandle_t light_queue;
extern QueueHandle_t sound_queue;
extern QueueHandle_t ultrasonic_queue;

// ========================= Task Handles =========================
extern TaskHandle_t temp_task_handle;
extern TaskHandle_t moisture_task_handle;
extern TaskHandle_t light_task_handle;
extern TaskHandle_t sound_task_handle;
extern TaskHandle_t ultrasonic_task_handle;

// ========================= Task Prototypes =========================
void temp_humidity_task(void *pv);
void moisture_task(void *pv);
void light_task(void *pv);
void sound_task(void *pv);
void ultrasonic_task(void *pv);

#endif

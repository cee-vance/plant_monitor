#ifndef CONTROL_LOOP_H
#define CONTROL_LOOP_H

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Task handle
extern TaskHandle_t control_task_handle;

// Prototype
void control_loop_task(void *pv);

#endif

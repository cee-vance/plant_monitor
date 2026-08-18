#pragma once

// ======================================================
//  Includes
// ======================================================
#include <Arduino.h>
#include "src/protocol.h"
#include "src/logging.h"

// ======================================================
//  Function Prototypes
// ======================================================

// ------------------------------------------------------
// Name: control_loop_task
// Description: FreeRTOS task that processes sensor data,
//              evaluates thresholds, and issues actuator
//              commands (vent control, watering triggers).
// ------------------------------------------------------
void control_loop_task(void *pvParameters);

// ------------------------------------------------------
// Name: run_threshold_watering_sequence
// Description: Executes the moisture-threshold watering
//              routine until moisture rises above the
//              configured threshold.
// ------------------------------------------------------
void run_threshold_watering_sequence();

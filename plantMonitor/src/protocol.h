#pragma once
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

// ======================================================
//  Protocol Modes
// ======================================================

enum control_mode_t {
    MODE_THRESHOLD = 0,
    MODE_SCHEDULE  = 1
};

// ======================================================
//  Config Message
// ======================================================

struct config_msg_t {
    control_mode_t mode;
    uint8_t        threshold_pct;
    uint32_t       schedule_hours;
};

// ======================================================
//  Protocol API
// ======================================================

// ISR (implemented in protocol.cpp)
extern void IRAM_ATTR cmd_isr();

// Task that applies config updates
void command_task(void *pvParameters);

// Low-level byte read helper
uint8_t read_cmd_byte_from_hw();


// handles setting up a new periodic watering schedule 
void protocol_apply_schedule(const config_msg_t &cfg);

void protocol_apply_threshold(const config_msg_t &cfg);



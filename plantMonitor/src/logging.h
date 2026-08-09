#pragma once
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "protocol.h"   // for control_mode_t

// ======================================================
//  Outgoing Plant Data Packet
// ======================================================

struct plant_data_t {
    uint32_t timestamp_ms;

    // raw sensor values
    uint16_t moisture_raw;
    float    moisture_pct;
    float    temperature_c;
    uint16_t light_raw;
    uint16_t distance_mm;

    // config values (current system state)
    control_mode_t mode;
    uint8_t        threshold_pct;
    uint32_t       schedule_hours;

    // actuator states
    bool pump_active;
    bool vent_open;
};

// Queue for outgoing plant data
extern QueueHandle_t plant_data_queue;

// ======================================================
//  Logging API (implemented in logging.cpp)
// ======================================================


// Push a plant_data_t packet into the queue
void logging_send_data(const plant_data_t &msg);

// Print formatted plant data (serial, BLE, etc.)
void logging_print_data(const plant_data_t &msg);


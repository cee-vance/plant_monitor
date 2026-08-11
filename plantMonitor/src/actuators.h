#pragma once
#include <Arduino.h>
#include "src/protocol.h"
#include "src/logging.h"

// ======================================================
//  Actuator Position Tracking
// ======================================================
extern long actuator_position_steps;

// ======================================================
//  Calibration Constants (update after calibration)
// ======================================================
static const int STEPS_PER_ML = 1500;        // your measured value
static const int FULL_STROKE_STEPS = 15000;  // 10mL syringe
static const int ACTUATOR_MIN_POS = 0;       // fully retracted
static const int ACTUATOR_MAX_POS = 15000;   // fully extended

// ======================================================
//  Stepper Control API
// ======================================================
void actuator_init();
void stepper_left(int steps);
void stepper_right(int steps);
void stepper_go_to(long target);

// High-level pump functions
void pump_dispense_ml(float ml);
void pump_draw_ml(float ml);


// ======================================================
//  Servo Control API
// ======================================================

// Servo vent control
static const int SERVO_PIN = 4;          // pick your actual pin
static const int SERVO_CHANNEL = 0;
static const int SERVO_TIMER = 0;

// Servo angles (tune these)
static const int VENT_OPEN_ANGLE = 90;
static const int VENT_CLOSE_ANGLE = 0;

// Servo API
void vent_init();
void vent_set_angle(int angle);
void vent_open();
void vent_close();

// called in control loop 
int compute_vent_angle(float current_temp, const config_msg_t &cfg);


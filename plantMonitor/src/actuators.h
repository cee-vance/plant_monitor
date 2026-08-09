#pragma once
#include <Arduino.h>

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

// High-level vent functions
void vent_open();
void vent_close();

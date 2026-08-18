#pragma once

// ======================================================
//  Includes
// ======================================================
#include <Arduino.h>
#include "driver/mcpwm.h"
#include "src/protocol.h"
#include "src/logging.h"
#include "timers.h"

// ======================================================
//  Macros / Constants
// ======================================================

// Stepper calibration (update after calibration)
constexpr int STEPS_PER_ML        = 1500;   // measured value
constexpr int FULL_STROKE_STEPS   = 15000;  // 10mL syringe
constexpr int ACTUATOR_MIN_POS    = 0;      // fully retracted
constexpr int ACTUATOR_MAX_POS    = 15000;  // fully extended

// Servo control
constexpr int SERVO_PIN           = 11;     // vent servo pin
constexpr int VENT_OPEN_ANGLE     = 90;
constexpr int VENT_CLOSE_ANGLE    = 0;

// ======================================================
//  Global Variables
// ======================================================
extern long actuator_position_steps;

// ======================================================
//  Function Prototypes
// ======================================================

// ------------------------------------------------------
// Name: actuator_init
// Description: Initializes stepper motor pins and resets
//              actuator position tracking.
// ------------------------------------------------------
void actuator_init();

// ------------------------------------------------------
// Name: stepper_left
// Description: Moves the stepper motor left by a number
//              of steps and updates position tracking.
// ------------------------------------------------------
void stepper_left(int steps);

// ------------------------------------------------------
// Name: stepper_right
// Description: Moves the stepper motor right by a number
//              of steps and updates position tracking.
// ------------------------------------------------------
void stepper_right(int steps);

// ------------------------------------------------------
// Name: stepper_go_to
// Description: Moves the actuator to an absolute target
//              position within allowed bounds.
// ------------------------------------------------------
void stepper_go_to(long target);

// ------------------------------------------------------
// Name: pump_dispense_ml
// Description: Dispenses water by converting mL to steps
//              and driving the actuator accordingly.
// ------------------------------------------------------
void pump_dispense_ml(float ml);

// ------------------------------------------------------
// Name: pump_draw_ml
// Description: Draws water into the syringe by converting
//              mL to steps and driving the actuator.
// ------------------------------------------------------
void pump_draw_ml(float ml);

// ------------------------------------------------------
// Name: vent_init
// Description: Initializes MCPWM for servo control and
//              moves vent through open/close sequence.
// ------------------------------------------------------
void vent_init();

// ------------------------------------------------------
// Name: vent_open
// Description: Sets servo PWM to open vent position.
// ------------------------------------------------------
void vent_open();

// ------------------------------------------------------
// Name: vent_close
// Description: Sets servo PWM to close vent position.
// ------------------------------------------------------
void vent_close();

// ------------------------------------------------------
// Name: update_vent_position
// Description: Applies temperature thresholds from config
//              to determine vent open/close behavior.
// ------------------------------------------------------
void update_vent_position(float current_temp, const config_msg_t &cfg);

// ------------------------------------------------------
// Name: schedule_timer_callback
// Description: Timer callback that queues scheduled water
//              events for the watering task.
// ------------------------------------------------------
void schedule_timer_callback(TimerHandle_t xTimer);

// ------------------------------------------------------
// Name: water_schedule_task
// Description: FreeRTOS task that dispenses water based
//              on scheduled events.
// ------------------------------------------------------
void water_schedule_task(void *pv);

// ------------------------------------------------------
// Name: manual_water_task
// Description: FreeRTOS task that handles manual watering
//              requests from user input.
// ------------------------------------------------------
void manual_water_task(void *pv);

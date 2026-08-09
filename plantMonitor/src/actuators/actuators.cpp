#include "src/actuators.h"
#include <driver/ledc.h>


// ===============================
//  Pin Definitions
// ===============================
#define IN1 10
#define IN2 11
#define IN3 12
#define IN4 13

// ===============================
//  Position Tracking
// ===============================
long actuator_position_steps = 0;

// ===============================
//  Half-step sequence
// ===============================
static const uint8_t seq[8][4] = {
    {1,0,0,0},
    {1,1,0,0},
    {0,1,0,0},
    {0,1,1,0},
    {0,0,1,0},
    {0,0,1,1},
    {0,0,0,1},
    {1,0,0,1}
};

// ===============================
//  Initialization
// ===============================
void actuator_init() {
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);

    actuator_position_steps = 0;
}

// ------------------------------------------------------
// Servo (Vent) Initialization
// ------------------------------------------------------
void vent_init() {
    // 50 Hz PWM for standard hobby servos
    const int freq = 50;
    const int resolution_bits = 16;   // gives 0–65535 duty range

    // ledcSetup(SERVO_CHANNEL, freq, resolution_bits);
    ledcAttach(SERVO_PIN, freq, resolution_bits);
}


// ===============================
//  Low-level stepping
// ===============================
static void stepMotor(int dir) {
    static int step = 0;

    step += dir;
    if (step < 0) step = 7;
    if (step > 7) step = 0;

    digitalWrite(IN1, seq[step][0]);
    digitalWrite(IN2, seq[step][1]);
    digitalWrite(IN3, seq[step][2]);
    digitalWrite(IN4, seq[step][3]);

    delay(3);  // speed control
}

void stepper_left(int steps) {
    for (int i = 0; i < steps; i++) {
        stepMotor(-1);
        actuator_position_steps--;
        if (actuator_position_steps < ACTUATOR_MIN_POS)
            actuator_position_steps = ACTUATOR_MIN_POS;
    }
}

void stepper_right(int steps) {
    for (int i = 0; i < steps; i++) {
        stepMotor(+1);
        actuator_position_steps++;
        if (actuator_position_steps > ACTUATOR_MAX_POS)
            actuator_position_steps = ACTUATOR_MAX_POS;
    }
}

// ===============================
//  Absolute positioning
// ===============================
void stepper_go_to(long target) {
    if (target < ACTUATOR_MIN_POS) target = ACTUATOR_MIN_POS;
    if (target > ACTUATOR_MAX_POS) target = ACTUATOR_MAX_POS;

    long delta = target - actuator_position_steps;

    if (delta > 0) stepper_right(delta);
    else if (delta < 0) stepper_left(-delta);
}

// ===============================
//  Pump control
// ===============================
void pump_dispense_ml(float ml) {
    int steps = (int)(ml * STEPS_PER_ML);
    stepper_right(steps);
}

void pump_draw_ml(float ml) {
    int steps = (int)(ml * STEPS_PER_ML);
    stepper_left(steps);
}

// ===============================
//  Vent control
// ===============================
void vent_open() {
    vent_set_angle(VENT_OPEN_ANGLE);
}

void vent_close() {
    vent_set_angle(VENT_CLOSE_ANGLE);
}



static uint32_t angle_to_duty(int angle) {
    // Clamp angle
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;

    // Convert angle to pulse width (1000–2000 µs)
    int pulse_us = 1000 + (angle * 1000) / 180;

    // Convert pulse width to duty cycle
    // 50 Hz → 20,000 µs period
    uint32_t duty = (pulse_us * 65535) / 20000;

    return duty;
}


void vent_set_angle(int angle) {
    uint32_t duty = angle_to_duty(angle);
    ledcWrite(SERVO_CHANNEL, duty);
}

// ------------------------------------------------------
// Compute vent angle from current temperature + config
// ------------------------------------------------------
int compute_vent_angle(float current_temp, const config_msg_t &cfg)
{
    // Clamp temperature to configured range
    float t = current_temp;
    if (t < cfg.temp_min) t = cfg.temp_min;
    if (t > cfg.temp_max) t = cfg.temp_max;

    // Map temperature → servo angle
    // temp_min  → VENT_CLOSE_ANGLE
    // temp_max  → VENT_OPEN_ANGLE
    float ratio = (t - cfg.temp_min) / (float)(cfg.temp_max - cfg.temp_min);

    int angle = (int)(VENT_CLOSE_ANGLE +
                      ratio * (VENT_OPEN_ANGLE - VENT_CLOSE_ANGLE));

    // Clamp final angle
    if (angle < VENT_CLOSE_ANGLE) angle = VENT_CLOSE_ANGLE;
    if (angle > VENT_OPEN_ANGLE)  angle = VENT_OPEN_ANGLE;

    return angle;
}




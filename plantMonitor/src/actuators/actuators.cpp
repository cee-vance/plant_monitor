#include "src/actuators.h"

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
    stepper_go_to(ACTUATOR_MAX_POS);
}

void vent_close() {
    stepper_go_to(ACTUATOR_MIN_POS);
}

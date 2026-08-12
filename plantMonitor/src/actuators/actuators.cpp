#include "src/actuators.h"
#include <driver/ledc.h>


// ===============================
//  Pin Definitions
// ===============================
#define IN1 18
#define IN2 19
#define IN3 20
#define IN4 21

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



// Calibrated pulse range for your servo (0–90°)
static const int MIN_US = 1000;   // 0°
static const int MID_US = 1500;   // 90°

// static inline int angleToUs(int angle) {
//     angle = constrain(angle, 0, 90);
//     return map(angle, 0, 90, MIN_US, MID_US);
// }

void vent_init() {
    // Route MCPWM0A output to the pin
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, SERVO_PIN);

    // Configure MCPWM timer
    mcpwm_config_t cfg;
    cfg.frequency = 50;
    cfg.cmpr_a = 0;
    cfg.counter_mode = MCPWM_UP_COUNTER;
    cfg.duty_mode = MCPWM_DUTY_MODE_0;

    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &cfg);

    // Move to closed position
    vent_close();
    delay(1000);
    vent_open();
    delay(1000);
    vent_close();
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
     mcpwm_set_duty_in_us(
            MCPWM_UNIT_0,
            MCPWM_TIMER_0,
            MCPWM_OPR_A,
            1500
        );
        Serial.println("Vent: OPEN");
}

void vent_close() {
         // Too cool → CLOSE vents
        mcpwm_set_duty_in_us(
            MCPWM_UNIT_0,
            MCPWM_TIMER_0,
            MCPWM_OPR_A,
            500
        );
        Serial.println("Vent: CLOSE");
}



// // static uint32_t angle_to_duty(int angle) {
// //     // Clamp angle
// //     if (angle < 0) angle = 0;
// //     if (angle > 180) angle = 180;

// //     // Convert angle to pulse width (1000–2000 µs)
// //     int pulse_us = 1000 + (angle * 1000) / 180;

// //     // Convert pulse width to duty cycle
// //     // 50 Hz → 20,000 µs period
// //     uint32_t duty = (pulse_us * 65535) / 20000;

// //     return duty;
// // }


// // void vent_set_angle(int angle) {
// //     uint32_t duty = angle_to_duty(angle);
// //     ledcWrite(SERVO_CHANNEL, duty);
// // }

// // ------------------------------------------------------
// // Compute vent angle from current temperature + config
// // ------------------------------------------------------
// int compute_vent_angle(float current_temp, const config_msg_t &cfg)
// {
//     // Clamp temperature to configured range
//     float t = current_temp;
//     if (t < cfg.temp_min) t = cfg.temp_min;
//     if (t > cfg.temp_max) t = cfg.temp_max;

//     // Map temperature → servo angle
//     // temp_min  → VENT_CLOSE_ANGLE
//     // temp_max  → VENT_OPEN_ANGLE
//     float ratio = (t - cfg.temp_min) / (float)(cfg.temp_max - cfg.temp_min);

//     int angle = (int)(VENT_CLOSE_ANGLE +
//                       ratio * (VENT_OPEN_ANGLE - VENT_CLOSE_ANGLE));

//     // Clamp final angle
//     if (angle < VENT_CLOSE_ANGLE) angle = VENT_CLOSE_ANGLE;
//     if (angle > VENT_OPEN_ANGLE)  angle = VENT_OPEN_ANGLE;

//     return angle;
// }





void update_vent_position(float current_temp, const config_msg_t &cfg) {

    // Ignore NaN readings completely
    if (isnan(current_temp)) {
        Serial.println("Vent: IGNORE (NaN)");
        return;
    }

    // Too warm → OPEN vents
    if (current_temp > cfg.temp_max) {
        vent_open();
        Serial.println("Vent: OPEN");
        return;
    }

    // Too cool → CLOSE vents
    if (current_temp < cfg.temp_min) {
        vent_close();
        Serial.println("Vent: CLOSE");
        return;
    }

    // In range → do nothing
    Serial.println("Vent: HOLD (in range)");
}


#include "src/actuators.h"
#include <driver/ledc.h>


// ===============================
//  Pin Definitions
// ===============================
#define IN1 16
#define IN2 17
#define IN3 18
#define IN4 8

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
    stepper_left(steps);
    Serial.println("DISPENSING WATER ON SCHEDULE!!! WOOHOO!");
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



void schedule_timer_callback(TimerHandle_t xTimer) {
    int qty = 1;
    xQueueSend(schedule_queue, &qty, 0);
}


void water_schedule_task(void *pv) {
    int qty;

    while (true) {
        if (xQueueReceive(schedule_queue, &qty, portMAX_DELAY)) {
            pump_dispense_ml(GLOBAL_SCHED_ML);   // SAFE HERE
        }
    }
}






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


// ======================================================
//  Includes
// ======================================================
#include "src/actuators.h"
#include <driver/ledc.h>

// ======================================================
//  Stepper Motor Pins
// ======================================================
#define IN1 16
#define IN2 17
#define IN3 18
#define IN4 8

// ======================================================
//  Position Tracking
// ======================================================
long actuator_position_steps = 0;

// ======================================================
//  Half-Step Sequence
// ======================================================
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

// ======================================================
// Name: actuator_init
// Description: Initializes stepper motor pins and resets
//              actuator position tracking.
// ======================================================
void actuator_init() {
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);

    actuator_position_steps = 0;
}

// ======================================================
// Name: vent_init
// Description: Initializes MCPWM for servo control and
//              performs an open/close calibration sweep.
// ======================================================
void vent_init() {
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, SERVO_PIN);

    mcpwm_config_t cfg;
    cfg.frequency     = 50;
    cfg.cmpr_a        = 0;
    cfg.counter_mode  = MCPWM_UP_COUNTER;
    cfg.duty_mode     = MCPWM_DUTY_MODE_0;

    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &cfg);

    vent_close();
    delay(1000);
    vent_open();
    delay(1000);
    vent_close();
}

// ======================================================
// Name: stepMotor
// Description: Performs one half-step in the given
//              direction using the stepper sequence.
// ======================================================
static void stepMotor(int dir) {
    static int step = 0;

    step += dir;
    if (step < 0) step = 7;
    if (step > 7) step = 0;

    digitalWrite(IN1, seq[step][0]);
    digitalWrite(IN2, seq[step][1]);
    digitalWrite(IN3, seq[step][2]);
    digitalWrite(IN4, seq[step][3]);

    delay(3);
}

// ======================================================
// Name: stepper_left
// Description: Moves the stepper motor left by a number
//              of steps and updates position tracking.
// ======================================================
void stepper_left(int steps) {
    for (int i = 0; i < steps; i++) {
        stepMotor(-1);
        actuator_position_steps--;
        if (actuator_position_steps < ACTUATOR_MIN_POS)
            actuator_position_steps = ACTUATOR_MIN_POS;
    }
}

// ======================================================
// Name: stepper_right
// Description: Moves the stepper motor right by a number
//              of steps and updates position tracking.
// ======================================================
void stepper_right(int steps) {
    for (int i = 0; i < steps; i++) {
        stepMotor(+1);
        actuator_position_steps++;
        if (actuator_position_steps > ACTUATOR_MAX_POS)
            actuator_position_steps = ACTUATOR_MAX_POS;
    }
}

// ======================================================
// Name: stepper_go_to
// Description: Moves the actuator to an absolute target
//              position within allowed bounds.
// ======================================================
void stepper_go_to(long target) {
    if (target < ACTUATOR_MIN_POS) target = ACTUATOR_MIN_POS;
    if (target > ACTUATOR_MAX_POS) target = ACTUATOR_MAX_POS;

    long delta = target - actuator_position_steps;

    if (delta > 0) stepper_right(delta);
    else if (delta < 0) stepper_left(-delta);
}

// ======================================================
// Name: pump_dispense_ml
// Description: Dispenses water by converting mL to steps
//              and driving the actuator.
// ======================================================
void pump_dispense_ml(float ml) {
    int steps = (int)(ml * STEPS_PER_ML);
    stepper_left(steps);
    Serial.println("DISPENSING WATER!!");
}

// ======================================================
// Name: pump_draw_ml
// Description: Draws water into the syringe by converting
//              mL to steps and driving the actuator.
// ======================================================
void pump_draw_ml(float ml) {
    int steps = (int)(ml * STEPS_PER_ML);
    stepper_left(steps);
}

// ======================================================
// Name: vent_open
// Description: Sets servo PWM to open vent position.
// ======================================================
void vent_open() {
    mcpwm_set_duty_in_us(
        MCPWM_UNIT_0,
        MCPWM_TIMER_0,
        MCPWM_OPR_A,
        1500
    );
    Serial.println("Vent: OPEN");
}

// ======================================================
// Name: vent_close
// Description: Sets servo PWM to close vent position.
// ======================================================
void vent_close() {
    mcpwm_set_duty_in_us(
        MCPWM_UNIT_0,
        MCPWM_TIMER_0,
        MCPWM_OPR_A,
        500
    );
    Serial.println("Vent: CLOSE");
}

// ======================================================
// Name: schedule_timer_callback
// Description: Timer callback that queues scheduled water
//              events for the watering task.
// ======================================================
void schedule_timer_callback(TimerHandle_t xTimer) {
    int qty = 1;
    xQueueSend(schedule_queue, &qty, 0);
}

// ======================================================
// Name: water_schedule_task
// Description: FreeRTOS task that dispenses water based
//              on scheduled events.
// ======================================================
void water_schedule_task(void *pv) {
    int qty;

    while (true) {
        if (xQueueReceive(schedule_queue, &qty, portMAX_DELAY)) {
            pump_dispense_ml(GLOBAL_SCHED_ML);
        }
    }
}

// ======================================================
// Name: update_vent_position
// Description: Applies temperature thresholds from config
//              to determine vent open/close behavior.
// ======================================================
void update_vent_position(float current_temp, const config_msg_t &cfg) {
    if (isnan(current_temp)) {
        Serial.println("Vent: IGNORE (NaN)");
        return;
    }

    if (current_temp > cfg.temp_max) {
        vent_open();
        Serial.println("Vent: OPEN");
        return;
    }

    if (current_temp < cfg.temp_min) {
        vent_close();
        Serial.println("Vent: CLOSE");
        return;
    }

    Serial.println("Vent: HOLD (in range)");
}

// ======================================================
// Name: manual_water_task
// Description: FreeRTOS task that performs manual watering
//              until a STOP notification is received.
// ======================================================
void manual_water_task(void *pv) {
    Serial.println("Manual watering task created, waiting for START");

    while (true) {
        uint32_t notifyValue = 0;

        xTaskNotifyWait(
            0,
            ULONG_MAX,
            &notifyValue,
            portMAX_DELAY
        );

        bool stopRequested = false;

        while (!stopRequested) {
            uint32_t stopValue = 0;

            if (xTaskNotifyWait(
                    0,
                    ULONG_MAX,
                    &stopValue,
                    0
                ) == pdTRUE)
            {
                stopRequested = true;
                Serial.println("Stop requested");
            }

            stepper_right(200);
            vTaskDelay(30);
        }
    }
}

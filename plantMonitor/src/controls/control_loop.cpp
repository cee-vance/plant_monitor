#include "control_loop.h"
#include "sensors.h"
#include "actuators.h"

// -----------------------------
// Global State
// -----------------------------
WaterMode currentMode = MODE_IDLE;

uint32_t schedule_hours = 0;
uint8_t quantity_ml = 0;
uint8_t threshold_pct = 0;

volatile bool water_due = false;

// Hardware timer handle
hw_timer_t *scheduleTimer = nullptr;

// -----------------------------
// Timer ISR
// -----------------------------
void IRAM_ATTR schedule_timer_isr() {
    water_due = true;
}

// -----------------------------
// Initialization
// -----------------------------
void control_loop_init() {
    // Timer created but not started yet
    scheduleTimer = timerBegin(0, 80, true); // 80 prescaler → 1us tick
    timerAttachInterrupt(scheduleTimer, &schedule_timer_isr, true);
}

// -----------------------------
// Configure schedule mode timer
// -----------------------------
void configure_schedule_timer(uint32_t hours) {
    uint64_t period_us = (uint64_t)hours * 3600000ULL * 1000ULL;

    timerAlarmWrite(scheduleTimer, period_us, true); // auto-reload
    timerAlarmEnable(scheduleTimer);
}

// -----------------------------
// Disable schedule timer
// -----------------------------
void disable_schedule_timer() {
    timerAlarmDisable(scheduleTimer);
    water_due = false;
}

// -----------------------------
// Watering routine
// -----------------------------
void perform_watering(uint8_t ml) {
    // Convert ml → actuator movement
    uint32_t duration_ms = ml * PUMP_MS_PER_ML; // defined in actuators.h

    pump_on();
    delay(duration_ms);
    pump_off();
}

// -----------------------------
// Vent control (always active)
// -----------------------------
void vent_control(float heat_index) {
    if (heat_index > HI_OPEN_THRESHOLD) {
        vent_open();
    } else {
        vent_close();
    }
}

// -----------------------------
// Main control loop
// -----------------------------
void control_loop_update() {
    // --- Read sensors ---
    SensorData data = read_all_sensors();
    DerivedMetrics m = compute_derived(data);

    // --- Vent always controlled ---
    vent_control(m.heat_index);

    // --- State machine ---
    switch (currentMode) {

        case MODE_IDLE:
            pump_off();
            break;

        case MODE_SCHEDULE:
            if (water_due) {
                water_due = false;
                perform_watering(quantity_ml);
            }
            break;

        case MODE_THRESHOLD:
            if (m.moist_pct < threshold_pct) {
                pump_on();
            } else {
                pump_off();
            }
            break;
    }
}

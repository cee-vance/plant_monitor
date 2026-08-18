#pragma once

// ======================================================
//  Includes
// ======================================================
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

// ======================================================
//  Macros / Constants
// ======================================================
constexpr int DEBOUNCE_MS = 25;

constexpr int BTN_UP    = 6;
constexpr int BTN_DOWN  = 4;
constexpr int BTN_ENTER = 5;

// ======================================================
//  Protocol Modes
// ======================================================
enum control_mode_t {
    MODE_THRESHOLD = 0,
    MODE_SCHEDULE  = 1
};

enum button_id_t {
    BTN_UP_ID,
    BTN_DOWN_ID,
    BTN_ENTER_ID
};

// ======================================================
//  Config Message
// ======================================================
struct config_msg_t {
    control_mode_t mode;
    uint8_t        threshold_pct;
    uint32_t       schedule_mins;

    // Vent control (temperature-based)
    uint8_t temp_min;   // start opening
    uint8_t temp_max;   // fully open
};

// ======================================================
//  Function Prototypes
// ======================================================

// ------------------------------------------------------
// Name: protocol_apply_schedule
// Description: Applies a new periodic watering schedule
//              based on the provided configuration.
// ------------------------------------------------------
void protocol_apply_schedule(const config_msg_t &cfg);

// ------------------------------------------------------
// Name: mode_mgr_task
// Description: FreeRTOS task that manages user interface
//              mode changes and configuration updates.
// ------------------------------------------------------
void mode_mgr_task(void *arg);

// ------------------------------------------------------
// Name: debounce
// Description: Performs button debounce logic for the
//              specified button and pin.
// ------------------------------------------------------
bool debounce(button_id_t id, int pin);

// ------------------------------------------------------
// Name: notify_mode_manager
// Description: Sends a notification bit to the mode
//              manager task (ISR → task communication).
// ------------------------------------------------------
void notify_mode_manager(uint32_t notifyBit);

// ------------------------------------------------------
// Name: isr_up
// Description: ISR for the UP button; handles debounce
//              and notifies mode manager.
// ------------------------------------------------------
void IRAM_ATTR isr_up();

// ------------------------------------------------------
// Name: isr_down
// Description: ISR for the DOWN button; handles debounce
//              and notifies mode manager.
// ------------------------------------------------------
void IRAM_ATTR isr_down();

// ------------------------------------------------------
// Name: isr_enter
// Description: ISR for the ENTER button; handles debounce
//              and notifies mode manager.
// ------------------------------------------------------
void IRAM_ATTR isr_enter();

// ------------------------------------------------------
// Name: schedule_timer_callback
// Description: Timer callback used for scheduled watering
//              events in schedule mode.
// ------------------------------------------------------
void schedule_timer_callback(TimerHandle_t xTimer);

// ------------------------------------------------------
// Name: attach_buttons
// Description: Attaches button interrupts for UP, DOWN,
//              and ENTER buttons.
// ------------------------------------------------------
void attach_buttons();

// ------------------------------------------------------
// Name: detach_buttons
// Description: Detaches button interrupts for UP, DOWN,
//              and ENTER buttons.
// ------------------------------------------------------
void detach_buttons();

// ------------------------------------------------------
// Name: prompt_confirm
// Description: Displays a confirmation prompt and returns
//              true/false based on user input.
// ------------------------------------------------------
bool prompt_confirm(const char* message);

// ------------------------------------------------------
// Name: run_config_wizard
// Description: Runs the interactive configuration wizard
//              for setting thresholds and schedule values.
// ------------------------------------------------------
void run_config_wizard();

// ------------------------------------------------------
// Name: prompt_for_int
// Description: Prompts user for an integer value within
//              a specified range.
// ------------------------------------------------------
int prompt_for_int(const char* prompt,
                   int initialValue,
                   int minVal,
                   int maxVal);

// ------------------------------------------------------
// Name: prompt_for_watering_mode
// Description: Prompts user to select watering mode
//              (threshold or schedule).
// ------------------------------------------------------
control_mode_t prompt_for_watering_mode(control_mode_t initialMode);

// ------------------------------------------------------
// Name: run_manual_water_prompt
// Description: Runs the manual watering prompt allowing
//              user-triggered watering events.
// ------------------------------------------------------
void run_manual_water_prompt();

// ======================================================
//  Global Variables
// ======================================================
extern volatile unsigned long lastDebounceUp;
extern volatile unsigned long lastDebounceDown;
extern volatile unsigned long lastDebounceEnter;

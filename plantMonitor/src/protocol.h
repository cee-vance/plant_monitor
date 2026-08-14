#ifndef PROTOCOL_H
#define PROTOCOL_H


#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#define DEBOUNCE_MS 25

#define BTN_UP 6
#define BTN_DOWN 4
#define BTN_ENTER 5

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
    uint8_t        temp_min;          // start opening
    uint8_t        temp_max;          // fully open
};


// ======================================================
//  Configuration
// ======================================================




// handles setting up a new periodic watering schedule 
void protocol_apply_schedule(const config_msg_t &cfg);




// configuration 

void mode_mgr_task(void* arg);


bool debounce(button_id_t id, int pin);

void  notify_mode_manager(uint32_t notifyBit) ;

void IRAM_ATTR isr_up(); 
void IRAM_ATTR isr_down(); 
void IRAM_ATTR isr_enter(); 

void schedule_timer_callback(TimerHandle_t xTimer) ;



extern volatile unsigned long lastDebounceUp;
extern volatile unsigned long lastDebounceDown;
extern volatile unsigned long lastDebounceEnter;


void attach_buttons();
void detach_buttons();


// ui stuff
bool prompt_confirm(const char* message) ;
void run_config_wizard();

int prompt_for_int(const char* prompt, int initialValue, int minVal, int maxVal);
control_mode_t prompt_for_watering_mode(control_mode_t initialMode) ;


void run_manual_water_prompt();

#endif






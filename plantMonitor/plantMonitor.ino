// ======================================================
//  Includes
// =========================================
=============
#include "freertos/FreeRTOSConfig.h"
#include "freertos/task.h"

// #include "src/globals.h"
#include "src/actuators.h"
#include "src/sensors.h"
#include "src/controls.h"
// #include "src/protocol.h"

// ======================================================
// Name: setup
// Description: Initializes hardware, creates queues, and
//              launches all FreeRTOS tasks.
// ======================================================
void setup() {
    Serial.begin(115200);

    // Hardware initialization
    actuator_init();
    vent_init();
    sensors_init();
    lcd_init();

    // ------------------------------------------------------
    // Queues
    // ------------------------------------------------------
    config_queue      = xQueueCreate(5, sizeof(config_msg_t));
    displayNowQueue   = xQueueCreate(8, sizeof(display_topic_t));
    plant_data_queue  = xQueueCreate(1, sizeof(plant_data_t));
    vent_queue        = xQueueCreate(1, sizeof(float));
    raw_queue         = xQueueCreate(1, sizeof(SensorData));
    schedule_queue    = xQueueCreate(4, sizeof(int));
    threshold_queue   = xQueueCreate(1, sizeof(int));
    sound_queue       = xQueueCreate(1, sizeof(sound_level_t));

    // ------------------------------------------------------
    // Tasks
    // ------------------------------------------------------

    // Sound disturbance task (Core 0)
    xTaskCreatePinnedToCore(
        sound_disturbance_task,
        "vibrationTask",
        4096,
        NULL,
        1,
        NULL,
        0
    );

    // Logging task
    xTaskCreatePinnedToCore(
        logging_task,
        "logging",
        4096,
        NULL,
        1,
        &logging_task_handle,
        1
    );

    // Sensor task
    xTaskCreatePinnedToCore(
        read_sensors_task,
        "sensors",
        4096,
        NULL,
        2,
        &read_sensors_handle,
        1
    );

    // Control loop task
    xTaskCreatePinnedToCore(
        control_loop_task,
        "control_loop",
        4096,
        NULL,
        2,
        &control_loop_handle,
        1
    );

    // Regular display rotation
    xTaskCreate(
        regular_display_task,
        "disp_reg",
        4096,
        NULL,
        1,
        &regularDisplayTaskHandle
    );

    // Alert display task
    xTaskCreate(
        display_now_task,
        "disp_now",
        4096,
        NULL,
        2,
        &displayNowTaskHandle
    );

    // Mode manager (UI)
    xTaskCreate(
        mode_mgr_task,
        "mode_mgr_task",
        4096,
        NULL,
        3,
        &mode_mgr_handle
    );

    // Manual watering task
    xTaskCreate(
        manual_water_task,
        "manualWater",
        2048,
        NULL,
        2,
        &manualWaterTaskHandle
    );

    // Scheduled watering task
    xTaskCreate(
        water_schedule_task,
        "water_sched",
        4096,
        NULL,
        3,
        NULL
    );

    // ------------------------------------------------------
    // Button setup
    // ------------------------------------------------------
    pinMode(BTN_DOWN,  INPUT_PULLUP);
    pinMode(BTN_ENTER, INPUT_PULLUP);
    pinMode(BTN_UP,    INPUT_PULLUP);

    attach_buttons();
}

// ======================================================
// Name: loop
// Description: Empty loop; all work is done in FreeRTOS
//              tasks.
// ======================================================
void loop() {
    // No work here — system is fully task-driven
}

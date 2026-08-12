



#include "freertos/FreeRTOSConfig.h"
#include "freertos/task.h"


#include "src/globals.h"
#include "src/actuators.h"
#include "src/sensors.h"
#include "src/controls.h"


void setup() {
    Serial.begin(115200);

    // start the acutators
    actuator_init();
    vent_init();

    sensors_init();

    lcd_init();

    // ======================================================
    // Create config queue (one struct per update)
    // ======================================================
    config_queue = xQueueCreate(
        5,                      // queue depth
        sizeof(config_msg_t)    // element size
    );

      displayNowQueue = xQueueCreate(8, sizeof(display_topic_t));

    // Create queue for plant_data_t packets
    plant_data_queue = xQueueCreate(1, sizeof(plant_data_t));

    vent_queue = xQueueCreate(1, sizeof(float));  // raw temp

    // ======================================================
    // Attach UART RX interrupt
    // ======================================================
    // ESP32-S3: attachInterruptArg is safe for Serial RX pin
    attachInterrupt(
        digitalPinToInterrupt(0),   // or your actual RX pin
        cmd_isr,
        FALLING                      // depends on your wiring
    );

    // ======================================================
    // Create command_task (applies config updates)
    // ======================================================
    xTaskCreatePinnedToCore(
        command_task,
        "cmd_task",
        4096,
        NULL,
        1,
        &command_task_handle,   // ← your handle
        1
    );




    // Create logging task
    xTaskCreatePinnedToCore(
        logging_task,          // task function
        "logging",             // name
        4096,                  // stack
        NULL,                  // params
        1,                     // priority (lowest)
        &logging_task_handle,  // handle
        1                      // core
    );

    // Create sensor task
    xTaskCreatePinnedToCore(
        read_sensors_task,
        "sensors",
        4096,
        NULL,
        2,                     // higher priority than logging
        &read_sensors_handle,
        1
    );

    xTaskCreatePinnedToCore(
    control_loop_task,        // task function
    "control_loop",           // name
    4096,                     // stack size
    NULL,                     // parameters
    2,                        // priority
    &control_loop_handle,     // ← store handle here
    1                         // run on core 1 (your actuator/sensor side)
    );

  

    xTaskCreate(regular_display_task,
                "disp_reg",
                4096,
                NULL,
                1,
                &regularDisplayTaskHandle);

    xTaskCreate(display_now_task,
                "disp_now",
                4096,
                NULL,
                2,
                &displayNowTaskHandle);



}

void loop() {
    // empty — all work done in tasks
}

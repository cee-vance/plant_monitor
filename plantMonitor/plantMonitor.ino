



#include "freertos/FreeRTOSConfig.h"
#include "freertos/task.h"


#include "src/globals.h"
#include "src/actuators.h"
#include "src/sensors.h"
#include "src/controls.h"
#include "src/protocol.h"


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

      raw_queue         = xQueueCreate(1, sizeof(SensorData));
    schedule_queue    = xQueueCreate(4, sizeof(int));

    threshold_queue = xQueueCreate(1, sizeof(int));

    sound_queue = xQueueCreate(1, sizeof(sound_level_t));



    xTaskCreatePinnedToCore(
            sound_disturbance_task,
            "vibrationTask",
            4096,
            NULL,
            1,
            NULL,
            0
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


    xTaskCreate(
            mode_mgr_task,        // task function
            "node_mgr_task",      // name
            4096,                 // stack size
            nullptr,              // parameter
            3,                    // priority (higher than logging)
            &mode_mgr_handle      // store handle here
        );

            // Create the manual watering task
    xTaskCreate(
        manual_water_task,        // task function
        "manualWater",            // name
        2048,                     // stack size
        NULL,                     // parameter
        2,                        // priority   
        &manualWaterTaskHandle    // handle out
    );



    // Attach UP b    pinMode(BTN_UP_PIN,    INPUT_PULLUP);
    pinMode(BTN_DOWN,  INPUT_PULLUP);
    pinMode(BTN_ENTER, INPUT_PULLUP);
    pinMode(BTN_UP, INPUT_PULLUP);

    attach_buttons();


    

    xTaskCreate(
        water_schedule_task,
        "water_sched",
        4096,
        NULL,
        3,
        NULL
    );



        

}

void loop() {
    // empty — all work done in tasks
}

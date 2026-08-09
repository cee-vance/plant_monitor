


#include "freertos/FreeRTOSConfig.h"
#include "freertos/task.h"


#include "src/globals.h"


void setup() {
    Serial.begin(9600);

        // ======================================================
    // Create config queue (one struct per update)
    // ======================================================
    config_queue = xQueueCreate(
        5,                      // queue depth
        sizeof(config_msg_t)    // element size
    );

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


}

void loop() {
    // empty — all work done in tasks
}

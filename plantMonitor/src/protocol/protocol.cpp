#include "src/protocol.h"
#include "src/globals.h"
#include <Arduino.h>

// ======================================================
//  Binary packet buffer (ISR fills this)
// ======================================================

static uint8_t rx_buf[sizeof(config_msg_t)];
static uint8_t rx_idx = 0;


// ======================================================
//  Read one byte from USART (Serial)
// ======================================================

uint8_t read_cmd_byte_from_hw() {
    return Serial.read();   // ISR-safe: interrupt fires only when data is available
}


// ======================================================
//  ISR: accumulate bytes until full config_msg_t received,
//       then queue the struct and reset buffer.
// ======================================================

void IRAM_ATTR cmd_isr() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // Read one byte from UART
    uint8_t b = read_cmd_byte_from_hw();

    // Store into buffer
    rx_buf[rx_idx++] = b;

    // Full struct received?
    if (rx_idx == sizeof(config_msg_t)) {

        // Reset index for next packet
        rx_idx = 0;

        // Convert raw bytes into a config_msg_t
        config_msg_t cfg;
        memcpy(&cfg, rx_buf, sizeof(cfg));

        // Push full struct to control loop
        xQueueSendFromISR(config_queue, &cfg, &xHigherPriorityTaskWoken);
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void command_task(void *pvParameters) {
    config_msg_t incoming;

    while (true) {
        // Wait forever for a new config message
        if (xQueueReceive(config_queue, &incoming, portMAX_DELAY)) {

            // Update the global config (should be volatile)
            memcpy((void*)&global_config, (void*)&incoming, sizeof(config_msg_t));

        }
    }
}



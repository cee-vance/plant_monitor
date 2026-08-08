#include "src/actuator_tasks.h"

void pump_task(void *pv) {
    actuator_msg_t cmd;

    pinMode(5, OUTPUT);  // example GPIO

    while (1) {
        if (xQueueReceive(pump_queue, &cmd, portMAX_DELAY)) {
            // Placeholder: turn pump on/off
            digitalWrite(5, cmd.value > 0.5 ? HIGH : LOW);
        }
    }
}

#include "src/actuator_tasks.h"

void fan_task(void *pv) {
    actuator_msg_t cmd;

    pinMode(7, OUTPUT);  // example GPIO

    while (1) {
        if (xQueueReceive(fan_queue, &cmd, portMAX_DELAY)) {
            digitalWrite(7, cmd.value > 0.5 ? HIGH : LOW);
        }
    }
}

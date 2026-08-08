#include "src/actuator_tasks.h"

void light_actuator_task(void *pv) {
    actuator_msg_t cmd;

    pinMode(6, OUTPUT);  // example GPIO

    while (1) {
        if (xQueueReceive(light_act_queue, &cmd, portMAX_DELAY)) {
            digitalWrite(6, cmd.value > 0.5 ? HIGH : LOW);
        }
    }
}

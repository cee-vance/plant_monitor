#include "control_loop.h"

TaskHandle_t control_task_handle = NULL;

void control_loop_task(void *pv) {
    while (1) {
        // placeholder control loop
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

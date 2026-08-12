
#include "src/globals.h"
#include "src/actuators.h"
#include "src/controls.h"

void control_loop_task(void *pvParameters)
{
    float temp_c;

    while (true)
    {
        if (xQueueReceive(vent_queue, &temp_c, portMAX_DELAY))
        {
            update_vent_position(temp_c, global_config);

            Serial.print("control_loop temp: ");
            Serial.println(temp_c);

            vTaskDelay(GLOBAL_TICK);
        }
    }
}


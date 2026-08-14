
#include "src/globals.h"
#include "src/actuators.h"
#include "src/controls.h"
#include "src/sensors.h"

void control_loop_task(void *pv) {
    int evt;

    while (true) {
        if (xQueueReceive(threshold_queue, &evt, portMAX_DELAY)) {

            Serial.println("RECEIVED THRESHOLD EVENT");
            run_threshold_watering_sequence();
        }
    }
}



void run_threshold_watering_sequence() {
    const int settle_ms = 1000; // water settles fast in glass demo

    while (true) {
        SensorData data;
        xQueueReceive(raw_queue, &data, portMAX_DELAY);

        if (data.moist_pct >= global_config.threshold_pct) {
            return; // done
        }

        pump_dispense_ml(GLOBAL_SCHED_ML);

        vTaskDelay(settle_ms / portTICK_PERIOD_MS);
    }
}



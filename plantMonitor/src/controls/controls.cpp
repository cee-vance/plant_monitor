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
    const int settle_ms = 400;   // moisture sensors need ~300–500ms to stabilize

    const float low  = global_config.threshold_pct - 3.0f;   // ON below this
    const float high = global_config.threshold_pct + 3.0f;   // OFF above this

    Serial.println("In threshold watering sequence");
    Serial.printf("Bringing water up to: %.2f\n", high);

    while (true) {
        SensorData data;
        xQueuePeek(raw_queue, &data, portMAX_DELAY);

        Serial.printf("Moisture Pct: %.2f\n", data.moist_pct);

        // Stop watering once moisture rises above hysteresis high band
        if (data.moist_pct >= high) {

            // Drain ALL pending threshold events
            int dummy;
            while (xQueueReceive(threshold_queue, &dummy, 0)) {}

            // Reset debounce flag
            threshold_active = false;

            Serial.println("Threshold watering complete.");
            return;
        }

        // Dispense water
        pump_dispense_ml(GLOBAL_SCHED_ML);

        vTaskDelay(settle_ms / portTICK_PERIOD_MS);
    }
}

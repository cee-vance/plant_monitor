#include "src/sensor_tasks.h"

// ========================= Tasks =========================

void temp_humidity_task(void *pv) {
    sensor_msg_t msg;
    while (1) {
        msg.timestamp = millis();
        msg.value = 0.0f;   // placeholder
        xQueueSend(temp_queue, &msg, 0);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void moisture_task(void *pv) {
    sensor_msg_t msg;
    while (1) {
        msg.timestamp = millis();
        msg.value = 0.0f;
        xQueueSend(moisture_queue, &msg, 0);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void light_task(void *pv) {
    sensor_msg_t msg;
    while (1) {
        msg.timestamp = millis();
        msg.value = 0.0f;
        xQueueSend(light_queue, &msg, 0);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void sound_task(void *pv) {
    sensor_msg_t msg;
    while (1) {
        msg.timestamp = millis();
        msg.value = 0.0f;
        xQueueSend(sound_queue, &msg, 0);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void ultrasonic_task(void *pv) {
    sensor_msg_t msg;
    while (1) {
        msg.timestamp = millis();
        msg.value = 0.0f;
        xQueueSend(ultrasonic_queue, &msg, 0);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
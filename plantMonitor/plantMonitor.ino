#include "src/sensor_tasks.h"
#include "src/actuator_tasks.h"
#include "src/control_loop.h"
#include "src/protocol.h"
#include "src/config.h"

#include "freertos/FreeRTOSConfig.h"
#include "freertos/task.h"




void setup() {
    Serial.begin(9600);

    // Create queues
    temp_queue       = xQueueCreate(10, sizeof(sensor_msg_t));
    moisture_queue   = xQueueCreate(10, sizeof(sensor_msg_t));
    light_queue      = xQueueCreate(10, sizeof(sensor_msg_t));
    sound_queue      = xQueueCreate(10, sizeof(sensor_msg_t));
    ultrasonic_queue = xQueueCreate(10, sizeof(sensor_msg_t));

    // Create tasks (Core 0)
    xTaskCreatePinnedToCore(temp_humidity_task, "temp", 4096, NULL, 1, &temp_task_handle, 0);
    xTaskCreatePinnedToCore(moisture_task,      "moist", 4096, NULL, 1, &moisture_task_handle, 0);
    xTaskCreatePinnedToCore(light_task,         "light", 4096, NULL, 1, &light_task_handle, 0);
    xTaskCreatePinnedToCore(sound_task,         "sound", 4096, NULL, 1, &sound_task_handle, 0);
    xTaskCreatePinnedToCore(ultrasonic_task,    "ultra", 4096, NULL, 1, &ultrasonic_task_handle, 0);

    // Create control loop (Core 1)
    xTaskCreatePinnedToCore(control_loop_task, "control", 8192, NULL, 2, &control_task_handle, 1);


    // Actuator queues
    pump_queue      = xQueueCreate(10, sizeof(actuator_msg_t));
    light_act_queue = xQueueCreate(10, sizeof(actuator_msg_t));
    fan_queue       = xQueueCreate(10, sizeof(actuator_msg_t));

    // Actuator tasks (Core 1)
    xTaskCreatePinnedToCore(pump_task,          "pump", 4096, NULL, 1, &pump_task_handle, 1);
    xTaskCreatePinnedToCore(light_actuator_task,"lightA",4096,NULL,1,&light_act_task_handle,1);
    xTaskCreatePinnedToCore(fan_task,           "fan", 4096, NULL, 1, &fan_task_handle, 1);

}

void loop() {
    // empty — all work done in tasks
}

#pragma once

#include "faja_config.h"

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

namespace faja {

extern QueueHandle_t colorQueue;
extern SystemState systemState;
extern uint32_t objectCount;
extern DetectedColor latestColor;
extern uint32_t blueObjectCount;
extern uint32_t greenObjectCount;
extern uint32_t redObjectCount;

WebStatus getWebStatus();
void createSystemTasks();
void taskControl(void *parameters);
void taskColorSensor(void *parameters);
void taskActuator(void *parameters);
void taskWebServer(void *parameters);

}  // namespace faja

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

void createSystemTasks();
void taskControl(void *parameters);
void taskColorSensor(void *parameters);
void taskActuator(void *parameters);
void taskWebServer(void *parameters);

}  // namespace faja

#pragma once

#include "faja_config.h"

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

namespace faja {

extern QueueHandle_t colorQueue;

void createSystemTasks();
void taskControl(void *parameters);
void taskColorSensor(void *parameters);
void taskActuator(void *parameters);

}  // namespace faja

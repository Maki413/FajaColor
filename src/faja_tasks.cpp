#include "faja_tasks.h"

#include "faja_hw.h"

namespace faja {

QueueHandle_t colorQueue = nullptr;

namespace {
bool systemRunning = false;
}

void taskControl(void *parameters) {
  (void)parameters;

  for (;;) {
    if (readStopButton()) {
      if (systemRunning) {
        setMotorEnabled(false);
        clearRgb();
        systemRunning = false;
      }
    }

    if (readStartButton() && !systemRunning) {
      setMotorEnabled(true);
      systemRunning = true;
    }

    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

void taskColorSensor(void *parameters) {
  (void)parameters;

  for (;;) {
    if (readIR()) {
      DetectedColor color = readColorFromSensor();
      xQueueSend(colorQueue, &color, portMAX_DELAY);
    }

    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

void taskActuator(void *parameters) {
  (void)parameters;

  DetectedColor color = DetectedColor::UNKNOWN;

  for (;;) {
    if (xQueueReceive(colorQueue, &color, portMAX_DELAY) == pdPASS) {
      moveServosForColor(color);
    }
  }
}

void createSystemTasks() {
  colorQueue = xQueueCreate(5, sizeof(DetectedColor));

  xTaskCreate(taskControl, "taskControl", 4096, nullptr, 2, nullptr);
  xTaskCreate(taskColorSensor, "taskColorSensor", 4096, nullptr, 3, nullptr);
  xTaskCreate(taskActuator, "taskActuator", 4096, nullptr, 2, nullptr);
}

}  // namespace faja

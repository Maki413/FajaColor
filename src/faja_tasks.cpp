#include "faja_tasks.h"

#include "faja_hw.h"

#include <WiFi.h>

namespace faja {

QueueHandle_t colorQueue = nullptr;
SystemState systemState = SystemState::STOPPED;
uint32_t objectCount = 0;
DetectedColor latestColor = DetectedColor::UNKNOWN;

namespace {

bool motorIsEnabled() {
  return systemState == SystemState::RUNNING || systemState == SystemState::WAITING_OUTPUT;
}

void setStoppedState() {
  setMotorEnabled(false);
  clearRgb();
  systemState = SystemState::STOPPED;
}

void setRunningState() {
  setMotorEnabled(true);
  systemState = SystemState::RUNNING;
}

void setAnalyzingState() {
  setMotorEnabled(false);
  systemState = SystemState::ANALYZING;
}

void setWaitingOutputState() {
  setMotorEnabled(true);
  systemState = SystemState::WAITING_OUTPUT;
}

}  // namespace

void taskWebServer(void *parameters) {
  (void)parameters;

  connectToWifi();

  if (WiFi.status() == WL_CONNECTED) {
    configureWebServer();
    Serial.printf("-> Web: http://%s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("-> Error: no se pudo conectar al WiFi");
  }

  for (;;) {
    if (WiFi.status() == WL_CONNECTED) {
      handleWebClient();
    }
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void taskControl(void *parameters) {
  (void)parameters;

  for (;;) {
    if (readStopButton() && systemState != SystemState::STOPPED) {
      setStoppedState();
    }

    if (readStartButton() && systemState == SystemState::STOPPED) {
      setRunningState();
    }

    vTaskDelay(pdMS_TO_TICKS(CONTROL_TASK_PERIOD_MS));
  }
}

void taskColorSensor(void *parameters) {
  (void)parameters;

  for (;;) {
    if (systemState == SystemState::RUNNING && readIR()) {
      const DetectedColor color = readColorFromSensor();
      if (color != DetectedColor::UNKNOWN) {
        latestColor = color;
        objectCount++;
        xQueueSend(colorQueue, &color, portMAX_DELAY);
        setAnalyzingState();
      }
    }

    vTaskDelay(pdMS_TO_TICKS(CONTROL_TASK_PERIOD_MS));
  }
}

void taskActuator(void *parameters) {
  (void)parameters;

  for (;;) {
    if (systemState == SystemState::ANALYZING) {
      DetectedColor color = DetectedColor::UNKNOWN;
      if (xQueueReceive(colorQueue, &color, 0) == pdPASS) {
        moveServosForColor(color);
        vTaskDelay(pdMS_TO_TICKS(COLOR_ANALYSIS_MS));
        clearRgb();
        setWaitingOutputState();
      }
    }

    if (systemState == SystemState::WAITING_OUTPUT && !readIR()) {
      setRunningState();
    }

    vTaskDelay(pdMS_TO_TICKS(CONTROL_TASK_PERIOD_MS));
  }
}

void createSystemTasks() {
  colorQueue = xQueueCreate(5, sizeof(DetectedColor));

  xTaskCreate(taskControl, "taskControl", 4096, nullptr, 2, nullptr);
  xTaskCreate(taskColorSensor, "taskColorSensor", 4096, nullptr, 3, nullptr);
  xTaskCreate(taskActuator, "taskActuator", 4096, nullptr, 2, nullptr);
  xTaskCreate(taskWebServer, "taskWebServer", 8192, nullptr, 1, nullptr);
}

}  // namespace faja

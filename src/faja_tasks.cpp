#include "faja_tasks.h"

#include "faja_hw.h"

#include <WiFi.h>

namespace faja {

QueueHandle_t colorQueue = nullptr;
SystemState systemState = SystemState::STOPPED;
uint32_t objectCount = 0;
DetectedColor latestColor = DetectedColor::UNKNOWN;
uint32_t blueObjectCount = 0;
uint32_t greenObjectCount = 0;
uint32_t redObjectCount = 0;

namespace {

portMUX_TYPE telemetryMux = portMUX_INITIALIZER_UNLOCKED;

SystemState currentSystemState() {
  portENTER_CRITICAL(&telemetryMux);
  const SystemState state = systemState;
  portEXIT_CRITICAL(&telemetryMux);
  return state;
}

void setStoppedState() {
  setMotorEnabled(false);
  clearRgb();
  portENTER_CRITICAL(&telemetryMux);
  systemState = SystemState::STOPPED;
  portEXIT_CRITICAL(&telemetryMux);
}

void setRunningState() {
  setMotorEnabled(true);
  portENTER_CRITICAL(&telemetryMux);
  systemState = SystemState::RUNNING;
  portEXIT_CRITICAL(&telemetryMux);
}

void setAnalyzingState() {
  setMotorEnabled(false);
  portENTER_CRITICAL(&telemetryMux);
  systemState = SystemState::ANALYZING;
  portEXIT_CRITICAL(&telemetryMux);
}

void setWaitingOutputState() {
  setMotorEnabled(true);
  portENTER_CRITICAL(&telemetryMux);
  systemState = SystemState::WAITING_OUTPUT;
  portEXIT_CRITICAL(&telemetryMux);
}

}  // namespace

WebStatus getWebStatus() {
  WebStatus status;
  portENTER_CRITICAL(&telemetryMux);
  status.objectCount = objectCount;
  status.blueCount = blueObjectCount;
  status.greenCount = greenObjectCount;
  status.redCount = redObjectCount;
  status.latestColor = latestColor;
  status.systemState = systemState;
  portEXIT_CRITICAL(&telemetryMux);
  status.rgbColor = getRgbLedColor();
  return status;
}

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
    if (readStopButton() && currentSystemState() != SystemState::STOPPED) {
      setStoppedState();
    }

    if (readStartButton() && currentSystemState() == SystemState::STOPPED) {
      setRunningState();
    }

    vTaskDelay(pdMS_TO_TICKS(CONTROL_TASK_PERIOD_MS));
  }
}

void taskColorSensor(void *parameters) {
  (void)parameters;

  for (;;) {
    if (currentSystemState() == SystemState::RUNNING && readIR()) {
      const DetectedColor color = readColorFromSensor();
      if (color != DetectedColor::UNKNOWN) {
        portENTER_CRITICAL(&telemetryMux);
        latestColor = color;
        objectCount++;
        switch (color) {
          case DetectedColor::BLUE:
            blueObjectCount++;
            break;
          case DetectedColor::GREEN:
            greenObjectCount++;
            break;
          case DetectedColor::RED:
            redObjectCount++;
            break;
          default:
            break;
        }
        portEXIT_CRITICAL(&telemetryMux);
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
    if (currentSystemState() == SystemState::ANALYZING) {
      DetectedColor color = DetectedColor::UNKNOWN;
      if (xQueueReceive(colorQueue, &color, 0) == pdPASS) {
        moveServosForColor(color);
        vTaskDelay(pdMS_TO_TICKS(COLOR_ANALYSIS_MS));
        clearRgb();
        setWaitingOutputState();
      }
    }

    if (currentSystemState() == SystemState::WAITING_OUTPUT && !readIR()) {
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

#include "faja_hw.h"
#include "faja_tasks.h"

void setup() {
  Serial.begin(115200);
  faja::hardwareBegin();
  faja::createSystemTasks();
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}
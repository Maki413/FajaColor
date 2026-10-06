#pragma once

#include "faja_config.h"

#include <ESP32Servo.h>
#include <String.h>

namespace faja {

extern SystemState systemState;

String buildWebPage(uint32_t objectCount, DetectedColor latestColor, bool motorEnabled);
String buildStatusJson(uint32_t objectCount, DetectedColor latestColor, bool motorEnabled);
void connectToWifi();
void configureWebServer();
void handleWebClient();

void hardwareBegin();
void setMotorEnabled(bool enabled);
void setRgbColor(bool red, bool green, bool blue);
void clearRgb();
bool readStartButton();
bool readStopButton();
bool readIR();
DetectedColor readColorFromSensor();
void moveServosForColor(DetectedColor color);

}  // namespace faja

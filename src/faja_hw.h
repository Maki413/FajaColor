#pragma once

#include "faja_config.h"

#include <ESP32Servo.h>

namespace faja {

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

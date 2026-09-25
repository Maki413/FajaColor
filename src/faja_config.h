#pragma once

#include <Arduino.h>

namespace faja {

constexpr int PIN_START = 34;
constexpr int PIN_STOP = 35;
constexpr int PIN_IR = 13;

constexpr int PIN_S0 = 16;
constexpr int PIN_S1 = 17;
constexpr int PIN_S2 = 5;
constexpr int PIN_S3 = 18;
constexpr int PIN_OUT = 19;

constexpr int PIN_IN1 = 27;
constexpr int PIN_IN2 = 26;

constexpr int PIN_LED_VERDE = 4;
constexpr int PIN_LED_ROJO = 15;

constexpr int PIN_SERVO1 = 14;
constexpr int PIN_SERVO2 = 12;

constexpr int PIN_RGB_R = 21;
constexpr int PIN_RGB_G = 22;
constexpr int PIN_RGB_B = 23;

constexpr int COLOR_MARGIN = 15;
constexpr uint32_t COLOR_ANALYSIS_MS = 2000;

enum class DetectedColor {
  UNKNOWN,
  BLUE,
  GREEN,
  RED
};

}  // namespace faja

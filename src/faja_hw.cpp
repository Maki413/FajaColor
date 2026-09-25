#include "faja_hw.h"

namespace faja {

namespace {
Servo servo1;
Servo servo2;

void setServoAngle(Servo& servo, int angle) {
  servo.write(angle);
}

}  // namespace

void hardwareBegin() {
  pinMode(PIN_START, INPUT);
  pinMode(PIN_STOP, INPUT);
  pinMode(PIN_IR, INPUT);

  pinMode(PIN_S0, OUTPUT);
  pinMode(PIN_S1, OUTPUT);
  pinMode(PIN_S2, OUTPUT);
  pinMode(PIN_S3, OUTPUT);
  pinMode(PIN_OUT, INPUT);

  digitalWrite(PIN_S0, HIGH);
  digitalWrite(PIN_S1, LOW);

  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);

  pinMode(PIN_LED_VERDE, OUTPUT);
  pinMode(PIN_LED_ROJO, OUTPUT);

  pinMode(PIN_RGB_R, OUTPUT);
  pinMode(PIN_RGB_G, OUTPUT);
  pinMode(PIN_RGB_B, OUTPUT);

  clearRgb();

  servo1.setPeriodHertz(50);
  servo2.setPeriodHertz(50);
  servo1.attach(PIN_SERVO1, 500, 2400);
  servo2.attach(PIN_SERVO2, 500, 2400);

  setServoAngle(servo1, 0);
  setServoAngle(servo2, 0);

  setMotorEnabled(false);
}

void setMotorEnabled(bool enabled) {
  if (enabled) {
    digitalWrite(PIN_IN1, HIGH);
    digitalWrite(PIN_IN2, LOW);
    digitalWrite(PIN_LED_VERDE, HIGH);
    digitalWrite(PIN_LED_ROJO, LOW);
  } else {
    digitalWrite(PIN_IN1, LOW);
    digitalWrite(PIN_IN2, LOW);
    digitalWrite(PIN_LED_VERDE, LOW);
    digitalWrite(PIN_LED_ROJO, HIGH);
  }
}

void setRgbColor(bool red, bool green, bool blue) {
  digitalWrite(PIN_RGB_R, red ? LOW : HIGH);
  digitalWrite(PIN_RGB_G, green ? LOW : HIGH);
  digitalWrite(PIN_RGB_B, blue ? LOW : HIGH);
}

void clearRgb() {
  digitalWrite(PIN_RGB_R, HIGH);
  digitalWrite(PIN_RGB_G, HIGH);
  digitalWrite(PIN_RGB_B, HIGH);
}

bool readStartButton() {
  return digitalRead(PIN_START) == HIGH;
}

bool readStopButton() {
  return digitalRead(PIN_STOP) == HIGH;
}

bool readIR() {
  return digitalRead(PIN_IR) == LOW;
}

DetectedColor readColorFromSensor() {
  digitalWrite(PIN_S2, LOW);
  digitalWrite(PIN_S3, LOW);
  unsigned long rPulse = pulseIn(PIN_OUT, LOW, 80000);

  digitalWrite(PIN_S2, LOW);
  digitalWrite(PIN_S3, HIGH);
  unsigned long bPulse = pulseIn(PIN_OUT, LOW, 80000);

  digitalWrite(PIN_S2, HIGH);
  digitalWrite(PIN_S3, HIGH);
  unsigned long gPulse = pulseIn(PIN_OUT, LOW, 80000);

  if (bPulse < rPulse - COLOR_MARGIN && bPulse < gPulse - COLOR_MARGIN) {
    return DetectedColor::BLUE;
  }

  if (gPulse < rPulse - COLOR_MARGIN && gPulse < bPulse - COLOR_MARGIN) {
    return DetectedColor::GREEN;
  }

  if (rPulse < gPulse - COLOR_MARGIN && rPulse < bPulse - COLOR_MARGIN) {
    return DetectedColor::RED;
  }

  return DetectedColor::UNKNOWN;
}

void moveServosForColor(DetectedColor color) {
  switch (color) {
    case DetectedColor::BLUE:
      setRgbColor(false, false, true);
      setServoAngle(servo1, 45);
      setServoAngle(servo2, 0);
      break;

    case DetectedColor::GREEN:
      setRgbColor(false, true, false);
      setServoAngle(servo1, 0);
      setServoAngle(servo2, 45);
      break;

    case DetectedColor::RED:
      setRgbColor(true, false, false);
      setServoAngle(servo1, 0);
      setServoAngle(servo2, 0);
      break;

    default:
      Serial.println("-> Color no determinado");
      clearRgb();
      setServoAngle(servo1, 0);
      setServoAngle(servo2, 0);
      break;
  }
}

}  // namespace faja

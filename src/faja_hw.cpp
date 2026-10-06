#include "faja_hw.h"

#include <WebServer.h>
#include <WiFi.h>

namespace faja {

extern uint32_t objectCount;
extern DetectedColor latestColor;

namespace {
Servo servo1;
Servo servo2;
WebServer webServer(80);

const char *colorName(DetectedColor color) {
  switch (color) {
    case DetectedColor::BLUE:
      return "Azul";
    case DetectedColor::GREEN:
      return "Verde";
    case DetectedColor::RED:
      return "Rojo";
    default:
      return "No detectado";
  }
}

const char *motorStateName(bool motorEnabled) {
  return motorEnabled ? "Encendido" : "Apagado";
}

bool motorIsEnabled() {
  return systemState == SystemState::RUNNING ||
         systemState == SystemState::WAITING_OUTPUT;
}

void handleRoot() {
  webServer.send(200, "text/html; charset=utf-8", buildWebPage(
      static_cast<uint32_t>(objectCount),
      latestColor,
      motorIsEnabled()));
}

void handleStatus() {
  webServer.send(200, "application/json", buildStatusJson(
      static_cast<uint32_t>(objectCount),
      latestColor,
      motorIsEnabled()));
}

void setServoAngle(Servo& servo, int angle) {
  servo.write(angle);
}

}  // namespace

String buildWebPage(uint32_t objectCount, DetectedColor latestColor, bool motorEnabled) {
  String page = R"(
<!doctype html>
<html lang="es">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Faja de Color</title>
  <style>
    :root { color-scheme: dark; font-family: Arial, sans-serif; }
    body { margin: 0; background: #111827; color: #f3f4f6; }
    main { max-width: 760px; margin: 0 auto; padding: 32px 20px; }
    h1 { text-align: center; }
    .cards { display: grid; grid-template-columns: repeat(auto-fit, minmax(220px, 1fr)); gap: 20px; }
    .card { padding: 24px; border-radius: 16px; background: #1f2937; box-shadow: 0 8px 24px rgba(0,0,0,.25); }
    .label { font-size: 0.85rem; color: #9ca3af; text-transform: uppercase; }
    .value { margin-top: 10px; font-size: clamp(2rem, 6vw, 3.5rem); font-weight: 700; }
    .status { color: #34d399; }
    .status.off { color: #f87171; }
    .refresh { margin-top: 24px; text-align: center; }
    .refresh a { color: #93c5fd; }
  </style>
</head>
<body>
  <main>
    <h1>Faja de Color</h1>
    <section class="cards">
      <article class="card">
        <div class="label">Objetos detectados</div>
        <div class="value" id="objectCount">)";
  page += String(objectCount);
  page += R"(</div>
      </article>
      <article class="card">
        <div class="label">Color detectado</div>
        <div class="value" id="color">)";
  page += colorName(latestColor);
  page += R"(</div>
      </article>
      <article class="card">
        <div class="label">Motor</div>
        <div class="value status)";
  page += motorEnabled ? "" : " off";
  page += R"(" id="motorStatus">)";
  page += motorStateName(motorEnabled);
  page += R"(</div>
      </article>
    </section>
    <div class="refresh"><a href="/">Actualizar</a></div>
  </main>
  <script>
    async function refresh() {
      const response = await fetch('/status');
      const data = await response.json();
      document.getElementById('objectCount').textContent = data.objectCount;
      document.getElementById('color').textContent = data.color;
      const motor = document.getElementById('motorStatus');
      motor.textContent = data.motor;
      motor.classList.toggle('off', data.motor === 'Apagado');
    }
    setInterval(refresh, 500);
  </script>
</body>
</html>
)";
  return page;
}

String buildStatusJson(uint32_t objectCount, DetectedColor latestColor, bool motorEnabled) {
  return String("{\"objectCount\":") + String(objectCount) +
         ",\"color\":\"" + colorName(latestColor) +
         "\",\"motor\":\"" + motorStateName(motorEnabled) + "\"}";
}

void connectToWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < WIFI_CONNECTION_TIMEOUT_MS) {
    delay(250);
  }
}

void configureWebServer() {
  webServer.on("/", HTTP_GET, handleRoot);
  webServer.on("/status", HTTP_GET, handleStatus);
  webServer.onNotFound([]() {
    webServer.send(404, "text/plain", "No encontrado");
  });
  webServer.begin();
}

void handleWebClient() {
  webServer.handleClient();
}

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

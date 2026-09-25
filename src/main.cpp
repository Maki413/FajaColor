#include <arduino.h>
#include <ESP32Servo.h>

// Botones
const int PIN_START = 34;
const int PIN_STOP  = 35;
// Sensor IR
const int PIN_IR = 13;
// SENSOR DE COLOR TCS3200
const int PIN_S0  = 16;
const int PIN_S1  = 17;
const int PIN_S2  = 5;
const int PIN_S3  = 18;
const int PIN_OUT = 19;
// DRIVER MOTOR L298N
const int PIN_IN1 = 27;
const int PIN_IN2 = 26;
// LEDs DE ESTADO DE LA FAJA
const int PIN_LED_VERDE = 4;
const int PIN_LED_ROJO  = 15;
// SERVOMOTORES
const int PIN_SERVO1 = 14;   // Azul
const int PIN_SERVO2 = 12;   // Verde
Servo servo1;
Servo servo2;
// LED RGB - ÁNODO COMÚN
const int PIN_RGB_R = 21;
const int PIN_RGB_G = 22;
const int PIN_RGB_B = 23;
// MÁQUINA DE ESTADOS

enum EstadoSistema {
  DETENIDO,
  AVANZANDO,
  ANALIZANDO,
  ESPERANDO_SALIDA
};

EstadoSistema estadoActual = DETENIDO;

// Timer del ESP32
hw_timer_t *timer = NULL;
volatile bool tiempoAnalisisCumplido = false;
const unsigned long TIEMPO_ANALISIS_US = 2000000;

// TOLERANCIA PARA DISCRIMINAR COLORES
const int MARGEN = 15;

// INTERRUPCIÓN DEL TIMER
void IRAM_ATTR timerCallback() {
  tiempoAnalisisCumplido = true;
}
void apagarRGB();
void colorRGB(int r, int g, int b);
void encenderMotor();
void apagarMotor();
void actualizarColorYServos();

void setup() {

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

  apagarRGB();

  servo1.setPeriodHertz(50);
  servo2.setPeriodHertz(50);

  servo1.attach(PIN_SERVO1, 500, 2400);
  servo2.attach(PIN_SERVO2, 500, 2400);

  servo1.write(0);
  servo2.write(0);

  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &timerCallback, true);
  timerAlarmWrite(timer, TIEMPO_ANALISIS_US, false);
  timerAlarmEnable(timer);
  timerStop(timer);

  apagarMotor();
}

void loop() {

  if (digitalRead(PIN_STOP) == HIGH) {

    if (estadoActual != DETENIDO) {

      // Detener motor
      apagarMotor();

      // Apagar RGB
      apagarRGB();

      // Regresar servos a 0°
      servo1.write(0);
      servo2.write(0);

      // Detener timer por seguridad
      timerStop(timer);

      // Limpiar bandera
      tiempoAnalisisCumplido = false;

      // Cambiar estado
      estadoActual = DETENIDO;
    }
  }

  if (digitalRead(PIN_START) == HIGH) {

    if (estadoActual == DETENIDO) {

      encenderMotor();

      estadoActual = AVANZANDO;
    }
  }

  // MÁQUINA DE ESTADOS

  switch (estadoActual) {

    case DETENIDO:
      // Esperar pulsación de START
      break;

    case AVANZANDO:

      if (digitalRead(PIN_IR) == LOW) {

        apagarMotor();
        actualizarColorYServos();

        tiempoAnalisisCumplido = false;
        timerStop(timer);
        timerWrite(timer, 0);
        timerAlarmWrite(timer, TIEMPO_ANALISIS_US, false);
        timerStart(timer);

        estadoActual = ANALIZANDO;
      }

      break;

    case ANALIZANDO:

      if (tiempoAnalisisCumplido) {

        tiempoAnalisisCumplido = false;

        apagarRGB();
        encenderMotor();
        estadoActual = ESPERANDO_SALIDA;
      }

      break;

    case ESPERANDO_SALIDA:

      if (digitalRead(PIN_IR) == HIGH) {

        estadoActual = AVANZANDO;
      }

      break;
  }
}

void encenderMotor() {

  digitalWrite(PIN_IN1, HIGH);
  digitalWrite(PIN_IN2, LOW);

  digitalWrite(PIN_LED_VERDE, HIGH);
  digitalWrite(PIN_LED_ROJO, LOW);
}

void apagarMotor() {

  digitalWrite(PIN_IN1, LOW);
  digitalWrite(PIN_IN2, LOW);

  digitalWrite(PIN_LED_VERDE, LOW);
  digitalWrite(PIN_LED_ROJO, HIGH);
}

void actualizarColorYServos() {

  digitalWrite(PIN_S2, LOW);
  digitalWrite(PIN_S3, LOW);

  unsigned long rPulse = pulseIn(PIN_OUT, LOW, 80000);

  digitalWrite(PIN_S2, LOW);
  digitalWrite(PIN_S3, HIGH);

  unsigned long bPulse = pulseIn(PIN_OUT, LOW, 80000);

  digitalWrite(PIN_S2, HIGH);
  digitalWrite(PIN_S3, HIGH);

  unsigned long gPulse = pulseIn(PIN_OUT, LOW, 80000);

  if (bPulse < rPulse - MARGEN &&
      bPulse < gPulse - MARGEN) {

    colorRGB(0, 0, 1);
    servo1.write(45);
    servo2.write(0);
  }

  else if (gPulse < rPulse - MARGEN &&
           gPulse < bPulse - MARGEN) {

    colorRGB(0, 1, 0);

    servo1.write(0);
    servo2.write(45);
  }

  else if (rPulse < gPulse - MARGEN &&
           rPulse < bPulse - MARGEN) {

    colorRGB(1, 0, 0);

    servo1.write(0);
    servo2.write(0);
  }

  else {

    Serial.println("-> Color no determinado");

    apagarRGB();

    servo1.write(0);
    servo2.write(0);
  }
}

void colorRGB(int r, int g, int b) {

  digitalWrite(PIN_RGB_R, r ? LOW : HIGH);
  digitalWrite(PIN_RGB_G, g ? LOW : HIGH);
  digitalWrite(PIN_RGB_B, b ? LOW : HIGH);
}

void apagarRGB() {

  digitalWrite(PIN_RGB_R, HIGH);
  digitalWrite(PIN_RGB_G, HIGH);
  digitalWrite(PIN_RGB_B, HIGH);
}
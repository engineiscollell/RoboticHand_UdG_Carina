#include <ESP32Servo.h>
#include <math.h>

const int emgPin = 34;
const int movA = 32;
const int movB = 26;
const int servoPin = 27;

Servo servo;

// Toggle + lockout
const unsigned long lockoutTime = 500;
unsigned long ultimTrigger = 0;
bool emgAnterior = false;
bool estatMoviment = false;

void aplicaEstat() {
  if (!estatMoviment) {
    digitalWrite(movA, LOW);
    digitalWrite(movB, LOW);
  } else {
    digitalWrite(movA, HIGH);
    digitalWrite(movB, LOW);
  }
}

// ====== Sliding window 60s via histogram (sobre v) ======
const unsigned long samplePeriodMs = 20;     // 50 Hz
const unsigned long windowMs = 60000;        // 1 minut
const int WIN = windowMs / samplePeriodMs;   // 3000 mostres

const int BINS = 128;
uint16_t hist[BINS] = {0};
uint8_t ringBin[WIN];
int ringIdx = 0;
bool ringPle = false;

// Rang esperat del senyal v (ajusta segons el teu mòdul)
// Si el teu v usa gairebé tot el rang ADC, pots posar 4095.
const int V_MIN = 0;
const int V_MAX = 4095;

// Map v -> bin
inline uint8_t vToBin(int v) {
  if (v <= V_MIN) return 0;
  if (v >= V_MAX) return BINS - 1;

  float r = (float)(v - V_MIN) / (float)(V_MAX - V_MIN); // 0..1
  int b = (int)(r * (BINS - 1) + 0.5f);
  if (b < 0) b = 0;
  if (b >= BINS) b = BINS - 1;
  return (uint8_t)b;
}

float binToV(int b) {
  // valor aproximat al centre del bin
  float step = (float)(V_MAX - V_MIN) / (float)BINS;
  return (float)V_MIN + ((float)b + 0.5f) * step;
}

int histCount() {
  return ringPle ? WIN : ringIdx;
}

float percentileFromHist(float p) {
  // p en [0..1]
  int n = histCount();
  if (n <= 0) return 0;

  int target = (int)roundf(p * (n - 1));
  int cum = 0;

  for (int b = 0; b < BINS; b++) {
    cum += hist[b];
    if (cum > target) return binToV(b);
  }
  return binToV(BINS - 1);
}

// Llindar robust
float thresholdV = 1000;      // inicial qualsevol raonable
const float kIQR = 2.0;       // 1.5..3
const float offsetThr = 10.0; // marge addicional

unsigned long lastSample = 0;
unsigned long lastThrUpdate = 0;
const unsigned long thrUpdatePeriodMs = 1000;

void recomputaLlindar() {
  float q1 = percentileFromHist(0.25f);
  float q3 = percentileFromHist(0.75f);
  float iqr = q3 - q1;
  if (iqr < 1.0f) iqr = 1.0f;

  thresholdV = q3 + kIQR * iqr + offsetThr;
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);

  pinMode(movA, OUTPUT);
  pinMode(movB, OUTPUT);

  servo.attach(servoPin);
  servo.write(90);

  aplicaEstat();
}

void loop() {
  unsigned long ara = millis();

  // Mostreig a 50 Hz
  if (ara - lastSample >= samplePeriodMs) {
    lastSample = ara;

    int v = analogRead(emgPin);

    // Update histograma finestra lliscant
    uint8_t b = vToBin(v);

    if (ringPle) {
      uint8_t old = ringBin[ringIdx];
      if (hist[old] > 0) hist[old]--;
    }
    ringBin[ringIdx] = b;
    hist[b]++;

    ringIdx++;
    if (ringIdx >= WIN) {
      ringIdx = 0;
      ringPle = true;
    }

    // Detecció flanc de pujada amb llindar adaptatiu
    bool emgActual = (v > thresholdV);

    if (emgActual && !emgAnterior && ((ara - ultimTrigger) > lockoutTime)) {
      ultimTrigger = ara;
      estatMoviment = !estatMoviment;
      aplicaEstat();

      Serial.println(estatMoviment ? "TOGGLE -> MOVIMENT" : "TOGGLE -> REPOS");
    }

    emgAnterior = emgActual;

    // Debug
    Serial.print("v:");
    Serial.print(v);
    Serial.print(" thr:");
    Serial.println(thresholdV, 1);
  }

  if (ara - lastThrUpdate >= thrUpdatePeriodMs) {
    lastThrUpdate = ara;
    recomputaLlindar();
  }
}
#include <ESP32Servo.h>
#include <math.h>

const int emgPin = 34;
const int movA = 32;
const int movB = 26;
const int servoPin = 27;

Servo servo;

// ====== Toggle + lockout ======
const unsigned long lockoutTime = 500;  // ms
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

// ====== Temporitzadors separats ======
// Lectura ràpida del sensor + detecció de trigger
const unsigned long readPeriodMs = 2;       // 500 Hz
unsigned long lastRead = 0;
int lastV = 0;  // últim valor llegit, compartit amb l'histograma

// Histograma lent (estadístiques)
const unsigned long histPeriodMs = 20;      // 50 Hz
unsigned long lastHist = 0;

// Recàlcul del llindar
const unsigned long thrUpdatePeriodMs = 1000;  // cada 1s
unsigned long lastThrUpdate = 0;

// ====== Cold start ======
// Temps mínim de dades abans de permetre triggers (ms)
const unsigned long coldStartMs = 60000;
unsigned long startTime = 0;

// ====== Sliding window 60s via histogram ======
const unsigned long windowMs = 60000;              // 1 minut
const int WIN = windowMs / histPeriodMs;           // 3000 mostres

const int BINS = 128;
uint16_t hist[BINS] = {0};
uint8_t ringBin[WIN];
int ringIdx = 0;
bool ringPle = false;

// Rang esperat del senyal v (ajusta segons el teu mòdul)
const int V_MIN = 0;
const int V_MAX = 2048;

// ====== Funcions histograma ======

inline uint8_t vToBin(int v) {
  if (v <= V_MIN) return 0;
  if (v >= V_MAX) return BINS - 1;

  float r = (float)(v - V_MIN) / (float)(V_MAX - V_MIN);
  int b = (int)(r * (BINS - 1) + 0.5f);
  if (b < 0) b = 0;
  if (b >= BINS) b = BINS - 1;
  return (uint8_t)b;
}

float binToV(int b) {
  float step = (float)(V_MAX - V_MIN) / (float)BINS;
  return (float)V_MIN + ((float)b + 0.5f) * step;
}

int histCount() {
  return ringPle ? WIN : ringIdx;
}

float percentileFromHist(float p) {
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

// ====== Llindar adaptatiu ======
float thresholdV = 1000;       // inicial (s'usarà fins que acabi el cold start)
const float kIQR = 5.0;
const float offsetThr = 50.0;
const float IQR_MIN = 20.0;

void recomputaLlindar() {
  float q1 = percentileFromHist(0.25f);
  float q3 = percentileFromHist(0.75f);
  float iqr = q3 - q1;
  if (iqr < IQR_MIN) iqr = IQR_MIN;

  thresholdV = q3 + kIQR * iqr + offsetThr;
}

// *********** SETUP ************
void setup() {
  Serial.begin(115200);
  analogReadResolution(12);

  pinMode(movA, OUTPUT);
  pinMode(movB, OUTPUT);

  servo.attach(servoPin);
  servo.write(90);

  aplicaEstat();

  startTime = millis();
  ultimTrigger = startTime - lockoutTime - 1;
}

// *********** LOOP ************
void loop() {
  unsigned long ara = millis();

  // ---- RÀPID (cada 2ms / 500Hz): lectura sensor + detecció trigger ----
  if (ara - lastRead >= readPeriodMs) {
    lastRead = ara;

    lastV = analogRead(emgPin);

    // Només permetre triggers si ja ha passat el cold start
    bool histReady = (ara - startTime) >= coldStartMs;

    bool emgActual = (lastV > thresholdV);

    if (histReady && emgActual && !emgAnterior && ((ara - ultimTrigger) > lockoutTime)) {
      ultimTrigger = ara;
      estatMoviment = !estatMoviment;
      aplicaEstat();

      // Serial.print(estatMoviment ? "TOGGLE -> MOVIMENT" : "TOGGLE -> REPOS");
      // Serial.print("  thr:");
      // Serial.println(thresholdV, 1);
    }

    emgAnterior = emgActual;

    // Debug
    // Serial.print("v:");
    // Serial.print(lastV);
    // Serial.print(" thr:");
    // Serial.println(thresholdV, 1);
  }

  // ---- LENT (cada 20ms / 50Hz): actualitzar histograma ----
  if (ara - lastHist >= histPeriodMs) {
    lastHist = ara;

    uint8_t b = vToBin(lastV);

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
  }

  // ---- RECÀLCUL LLINDAR (cada 1s) ----
  if (ara - lastThrUpdate >= thrUpdatePeriodMs) {
    lastThrUpdate = ara;
    recomputaLlindar();
  }
}

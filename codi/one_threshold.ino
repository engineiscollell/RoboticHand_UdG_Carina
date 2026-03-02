#include <ESP32Servo.h>

// Pins ESP32
const int emgPin = 34; // Entrada senyal EMG
const int movA = 32;   // Sortida moviment A
const int movB = 26;   // Sortida moviment B
const int servoPin = 27;

Servo servo;

// Constants senyal EMG (ajustar segons proves)
const int llindar = 600;
const unsigned long lockoutTime = 500; // ms

// Control detecció
unsigned long ultimTrigger = 0;
bool emgAnterior = false;

// Estat: false = repòs, true = moviment
bool estatMoviment = false;

bool esMajorQueLlindar(int valor) {
  return valor > llindar;
}

void aplicaEstat() {
  if (!estatMoviment) {
    // REPÒS
    digitalWrite(movA, LOW);
    digitalWrite(movB, LOW);
  } else {
    digitalWrite(movA, HIGH);
    digitalWrite(movB, LOW);
  }
}

// *********** SETUP ************
void setup() {
  Serial.begin(115200);
  analogReadResolution(12); // ESP32

  pinMode(movA, OUTPUT);
  pinMode(movB, OUTPUT);

  servo.attach(servoPin);
  servo.write(90);

  // Estat inicial = repòs
  aplicaEstat();
}

// *********** LOOP ************
void loop() {
  int v = analogRead(emgPin);
  Serial.println(v);

  bool emgActual = esMajorQueLlindar(v);
  unsigned long ara = millis();

  // Detectem flanc de pujada amb lockout
  if (emgActual && !emgAnterior && ((ara - ultimTrigger) > lockoutTime)) {
    ultimTrigger = ara;

    // Toggle entre repòs i moviment
    estatMoviment = !estatMoviment;
    aplicaEstat();

    Serial.print("Nou estatMoviment: ");
    Serial.println(estatMoviment ? "MOVIMENT" : "REPOS");
  }

  emgAnterior = emgActual;
  delay(1); // si es vol encara més responsiu, es pot treure o pujar-lo una mica segons soroll
}
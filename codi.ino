// A partir del senyal d'un sol sensor, crear 2 moviments diferents
// Extra: Provar fer interrupcions per detectar pics durant un temps

#include <ESP32Servo.h>

// Pins ESP32
const int emgPin = 34; // Entrada senyal EMG
const int movA = 32;   // Sortida moviment A
const int movB = 26;   // Sortida moviment B
const int servoPin = 27;

Servo servo;

// Constants servomotors
bool servoActiu = false; // true = girant, false = aturat
bool sentit = true;      // true = sentit A, false = sentit B

// Variables per distingir moviments
int cnt = 0;                              // Comptar els polsos per identificar moviment desitjat
const unsigned long finestraTemps = 3000; // ms (es pot ajustar)

// Constants senyal EMG (s'han d'ajustar cada cop que es facin proves)
const int llindar = 600;
const unsigned long lockoutTime = 500; // ms

// Constants control
unsigned long ultimPols = 0;
bool emgAnterior = false;

/*
// Declaracio interrupcio
void intPics() {
  contES++;
}
*/

bool esMajorQueLlindar(int valor) {
  return valor > llindar;
}

// *********** SETUP ************
void setup()
{
    Serial.begin(115200);
    analogReadResolution(12); // ESP32

    pinMode(movA, OUTPUT);
    pinMode(movB, OUTPUT);

    servo.attach(servoPin);
    servo.write(90);

    // attachInterrupt(digitalPinToInterrupt(emgPin), intPics, RISING);
}

// *********** LOOP ************
void loop()
{
    int v = analogRead(emgPin);
    Serial.println(v);

    bool emgActual = esMajorQueLlindar(v);
    unsigned long ara = millis();

    // Detectem flancs de pujada (ha d'haver passat prou temps)
    if (emgActual && !emgAnterior && ((ara - ultimPols) > lockoutTime))
    {
        ultimPols = ara;
        cnt = 1; 
        unsigned long inici = ara;

        while ((millis() - inici) < finestraTemps)
        {
            v = analogRead(emgPin);
            bool emgNou = esMajorQueLlindar(v);
            unsigned long t = millis();
            // Mentre duri el temps anem comptant polsos

            if (emgNou && !emgActual && ((t - ultimPols) > lockoutTime)) {
                cnt++;
                ultimPols = t;
            }

            emgActual = emgNou;
            delay(5);
        }
    }

    Serial.print("Polsos detectats: ");
    Serial.println(cnt);

    // Passat el temps de deteccio de flancs, identifiquem quin moviment s'ha d'executar
    if (cnt == 1)
    {
        // Es vol fer movA
        digitalWrite(movA, HIGH);
        digitalWrite(movB, LOW);
    }
    else if (cnt == 2)
        // Es vol fer movB
        digitalWrite(movA, LOW);
        digitalWrite(movB, HIGH);
    {
    }
    else 
    {
        // Es vol fer movB
        digitalWrite(movA, LOW);
        digitalWrite(movB, LOW);
    }

    cnt = 0;

    emgAnterior = emgActual;
    delay(1); // Els delays poden ser problematics.
}

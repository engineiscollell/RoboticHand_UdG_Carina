// A partir del senyal d'un sol sensor, crear 2 moviments diferents
// Extra: Provar fer interrupcions per detectar pics durant un temps

#include <ESP32Servo.h>

// Pins ESP32
const int emgPin = 34; // Entrada senyal EMG
const int movA = 32;   // Sortida moviment A
const int movB = 26;   // Sortida moviment B
const int servoPin = 27;

Servo servo;

// Constants senyal EMG (s'han d'ajustar cada cop que es facin proves)
const int llindar1 = 1000;
const int llindar2 = 2000;
const unsigned long lockoutTime = 500; // ms

// Constants control
unsigned long ultimEvent = 0;
bool armT1 = true;
bool armT2 = true;

enum Estat
{
    REPOS,
    POS1,
    POS2
};
Estat estat = REPOS;

void aplicaEstat(Estat e)
{
    if (e == POS1)
    {
        digitalWrite(movA, HIGH);
        digitalWrite(movB, LOW);
    }
    else if (e == POS2)
    {
        digitalWrite(movA, LOW);
        digitalWrite(movB, HIGH);
    }
    else
    {
        digitalWrite(movA, LOW);
        digitalWrite(movB, LOW);
    }
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

    aplicaEstat(REPOS);

    // attachInterrupt(digitalPinToInterrupt(emgPin), intPics, RISING);
}

// *********** LOOP ************
void loop()
{
    int v = analogRead(emgPin);
    Serial.println(v);

    unsigned long ara = millis();

    bool superaT1 = (v >= llindar1);
    bool superaT2 = (v >= llindar2);

    if (!superaT1)
        armT1 = true;
    if (!superaT2)
        armT2 = true;

    bool eventT2 = superaT2 && armT2 && ((ara - ultimEvent) > lockoutTime);
    bool eventT1 = superaT1 && armT1 && ((ara - ultimEvent) > lockoutTime) && !eventT2;

    if (eventT2)
    {
        armT2 = false;
        ultimEvent = ara;

        Serial.println(">>> EVENT T2!");

        // Regles amb T2
        if (estat == REPOS) {
            estat = POS2;
        }
        else if (estat == POS1) {
            estat = POS2;
        }
        else if (estat == POS2) {
            estat = REPOS;
        }
    }
    else if (eventT1)
    {
        armT1 = false;
        ultimEvent = ara;

        Serial.println(">>> EVENT T1!");

        // Regles amb T1
        if (estat == REPOS) {
            estat = POS1;
        }
        else if (estat == POS1) {
            estat = REPOS;
        }
        else if (estat == POS2) {
            estat = POS1;
        }
    }

    aplicaEstat(estat);

    delay(1);
}

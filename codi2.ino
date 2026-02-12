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

// temps màxim per decidir si el que sembla T1 acaba sent T2
const unsigned long DECISION_MS = 100;
bool pending = false;
unsigned long tPending = 0;

const unsigned long lockoutTime = 250; // ms

// Constants control
unsigned long ultimEvent = 0;
bool arm = true;

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

void aplicaEventT1()
{
    if (estat == REPOS)
    {
        estat = POS1;
    }
    else if (estat == POS1)
    {
        estat = REPOS;
    }
    else if (estat == POS2)
    {
        estat = POS1;
    }
}

void aplicaEventT2()
{
    if (estat == REPOS)
    {
        estat = POS2;
    }
    else if (estat == POS1)
    {
        estat = POS2;
    }
    else if (estat == POS2)
    {
        estat = REPOS;
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
    {
        arm = true;

        if (pending)
        {
            aplicaEventT1();
            ultimEvent = ara;
            pending = false;
            arm = false;

            Serial.print("EVENT T1 (cau sota T1) ");
            aplicaEstat(estat);
        }
    }

    // Si no estem en mode decisió, i detectem T1 (armats), entrem en pending
    if (!pending && arm && superaT1 && ((ara - ultimEvent) > lockoutTime))
    {
        pending = true;
        tPending = ara;
        arm = false;
    }

    if (pending) {
        // Si durant la finestra arriba a T2, guanya T2 immediatament
        if (superaT2) {
            aplicaEventT2();
            ultimEvent = ara;
            pending = false;

            Serial.print("EVENT T2");
            aplicaEstat(estat);
        }
        else if ((ara - tPending) >= DECISION_MS) {
            aplicaEventT1();
            ultimEvent = ara;
            pending = false;

            Serial.print("EVENT T1");
            aplicaEstat(estat);
        }
    }

    delay(1);
}

const int EMG_PIN = 34;
int v = 0;


void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  analogSetPinAttenuation(EMG_PIN, ADC_11db);
}

void loop() {
  v = analogRead(EMG_PIN);
  Serial.println(v);
  delay(15);
}

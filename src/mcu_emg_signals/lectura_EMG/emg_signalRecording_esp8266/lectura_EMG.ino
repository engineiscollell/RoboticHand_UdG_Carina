const int emgPin = A0;      // Changed from 34 to A0 for ESP8266
bool isRecording = false;   // Is the ESP8266 currently sending data?
String label = "None";      // The name of the movement
int repNumber = 0;          // Counts how many times you've recorded

void setup() {
  Serial.begin(115200);
  Serial.println("--- EMG DATA COLLECTOR (ESP8266) ---");
  Serial.println("Commands: 'r' = Rest, 't' = Tripod, 's' = Stop");
}

void loop() {
  // Check if you typed something on the PC
  if (Serial.available() > 0) {
    char command = Serial.read();

    if (command == 'r') {
      label = "Rest";
      repNumber++;
      isRecording = true;
      Serial.println("# STARTING: " + label + " (Rep " + String(repNumber) + ")");
    } 
    else if (command == 't') {
      label = "Tripod_Grasp";
      repNumber++;
      isRecording = true;
      Serial.println("# STARTING: " + label + " (Rep " + String(repNumber) + ")");
    } 
    else if (command == 's') {
      isRecording = false;
      Serial.println("# STOPPED");
    }
  }

  // If we are in "recording mode", send data to the PC
  if (isRecording) {
    int sensorValue = analogRead(emgPin); // Returns 0-1023
    
    // Format: Time, Value, Label, Repetition
    Serial.print(millis());
    Serial.print(",");
    Serial.print(sensorValue);
    Serial.print(",");
    Serial.print(label);
    Serial.print(",");
    Serial.println(repNumber);

    // Note: ESP8266 background tasks (WiFi) like a tiny bit of breathing room.
    // delay(2) is fine, but yield() is also helpful for stability.
    delay(10); 
  }
}
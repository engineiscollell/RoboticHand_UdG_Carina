// For ESP32, Pin 34 is a great choice as it is an Input-Only ADC pin
const int emgPin = 34;      
bool isRecording = false;   
String label = "None";      
int repNumber = 0;          

void setup() {
  // ESP32 handles high baud rates easily
  Serial.begin(115200);
  
  // Set the resolution to 12-bit (0-4095)
  // You can also use 10-bit (0-1023) if you want it to match the ESP8266 exactly
  analogReadResolution(12); 

  Serial.println("--- EMG DATA COLLECTOR (ESP32) ---");
  Serial.println("Commands: 'r' = Rest, 't' = Tripod, 's' = Stop");
}

void loop() {
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

  if (isRecording) {
    // ESP32 reading (default 0-4095)
    int sensorValue = analogRead(emgPin); 
    
    Serial.print(millis());
    Serial.print(",");
    Serial.print(sensorValue);
    Serial.print(",");
    Serial.print(label);
    Serial.print(",");
    Serial.println(repNumber);

    // Faster sampling is possible on ESP32, but 10ms is stable for basic monitoring
    delay(10); 
  }
}
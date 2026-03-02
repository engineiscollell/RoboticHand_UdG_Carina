// EMG input pin (ESP32 ADC pin)
#define EMG_PIN 34        // ADC1 channel (safe pin)

// Serial baud rate
#define BAUD_RATE 115200

// Recording duration
#define RECORDING_TIME_MS 5000   // 5 seconds

// Sampling interval
#define SAMPLE_INTERVAL_US 1000  // 1kHz sampling

// Buffer size
#define MAX_SAMPLES 6000

// Buffers
uint16_t emgBuffer[MAX_SAMPLES];
uint32_t timeBuffer[MAX_SAMPLES];
uint32_t sampleCount = 0;

// Labels
char signalType = 'N';   // R or T
char qualityLabel = 'N'; // G or B

// State
bool isRecording = false;
bool waitingForQuality = false;

// Timing
uint32_t startTimeMs = 0;
uint32_t lastSampleUs = 0;

void setup() {
  Serial.begin(BAUD_RATE);
  delay(1000);

  // ESP32 ADC config
  analogReadResolution(12);        // 12-bit ADC (0–4095)
  analogSetAttenuation(ADC_11db);  // full range ~3.3V

  Serial.println("\n=== ESP32 EMG SERIAL RECORDER ===");
  Serial.println("Commands:");
  Serial.println("R  -> Record REST (5s)");
  Serial.println("T  -> Record TRIPOD (5s)");
  Serial.println("--------------------------------");
  Serial.println("After recording:");
  Serial.println("G -> Good recording");
  Serial.println("B -> Bad recording");
  Serial.println("================================");
}

void loop() {

  /* ===== USER COMMAND ===== */
  if (!isRecording && !waitingForQuality && Serial.available()) {
    char cmd = Serial.read();

    if (cmd == 'R' || cmd == 'r') {
      signalType = 'R';
      startRecording();
    }

    if (cmd == 'T' || cmd == 't') {
      signalType = 'T';
      startRecording();
    }
  }

  /* ===== RECORDING MODE ===== */
  if (isRecording) {
    uint32_t nowUs = micros();

    if (nowUs - lastSampleUs >= SAMPLE_INTERVAL_US) {
      lastSampleUs = nowUs;

      if (sampleCount < MAX_SAMPLES) {
        emgBuffer[sampleCount] = analogRead(EMG_PIN);
        timeBuffer[sampleCount] = millis();
        sampleCount++;
      }
    }

    // Stop after fixed time
    if (millis() - startTimeMs >= RECORDING_TIME_MS) {
      isRecording = false;
      waitingForQuality = true;

      Serial.println("\n--- Recording Finished ---");
      Serial.println("Is this recording Good or Bad?");
      Serial.println("Type G or B");
    }
  }

  /* ===== QUALITY LABEL ===== */
  if (waitingForQuality && Serial.available()) {
    char q = Serial.read();

    if (q == 'G' || q == 'g' || q == 'B' || q == 'b') {
      qualityLabel = toupper(q);
      printCSV();
      resetSystem();
    }
  }
}


void startRecording() {
  Serial.println("\n--- Recording Started ---");
  Serial.print("Signal Type: ");
  if (signalType == 'R') Serial.println("REST");
  if (signalType == 'T') Serial.println("TRIPOD");

  sampleCount = 0;
  startTimeMs = millis();
  lastSampleUs = micros();
  isRecording = true;
  waitingForQuality = false;
}

void printCSV() {
  Serial.println("\n===== CSV DATA START =====");
  Serial.println("time_ms,emg_value,signal_type,quality");

  for (uint32_t i = 0; i < sampleCount; i++) {
    Serial.print(timeBuffer[i]);
    Serial.print(",");
    Serial.print(emgBuffer[i]);
    Serial.print(",");

    if (signalType == 'R') Serial.print("Rest");
    if (signalType == 'T') Serial.print("Tripod");

    Serial.print(",");

    if (qualityLabel == 'G') Serial.println("Good");
    if (qualityLabel == 'B') Serial.println("Bad");
  }

  Serial.println("===== CSV DATA END =====\n");
}


void resetSystem() {
  sampleCount = 0;
  signalType = 'N';
  qualityLabel = 'N';
  waitingForQuality = false;

  Serial.println("Ready for next recording.");
  Serial.println("Press R (Rest) or T (Tripod)");
}
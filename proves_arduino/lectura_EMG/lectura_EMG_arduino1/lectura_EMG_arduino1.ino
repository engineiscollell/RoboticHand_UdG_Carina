// Signal recording
// PARAMETERS:
int signalTime = 5; //s
char goodSignal = 'g';
char badSignal = 'b';
char tripodRecording = 't';
char restRecording = 'r';
int delayTimer = 10;
 
bool isRecording = false;   
String label = "None";    

int repNumberRest = 0;          
int repNumberTripod = 0;  
int timer = 0;    

const int emgPin = A0;     

void setup() {
  Serial.begin(9600);

  Serial.println("Gravació de senyals");
  Serial.println("Commands: '" + restRecording + "' = Rest, '" + tripodRecording + "' = Tripod/  ");
  Serial.println("Commands: '" + goodSignal + "' = Good Recording, '" + badSignal + "' = Bad R/  ");
}

void loop() {
  // 1. SEMPRE comprovem si l'usuari ha enviat una tecla nova
  if (Serial.available() > 0) {
    char incomingByte = Serial.read();

    // Si NO estem gravant, busquem 'r' o 't'
    if (!isRecording) {
      if (incomingByte == 'r') {
        label = "Rest";
        repNumberRest++;
        isRecording = true;
        timer = 0; // Reiniciem el comptador de temps
      } 
      else if (incomingByte == 't') {
        label = "Tripod_Grasp";
        repNumberTripod++;
        isRecording = true;
        timer = 0;
      }
    } 
    // Si JA estem gravant, busquem 'g' o 'b' per acabar
    else {
      if (incomingByte == goodSignal) {
        Serial.println("-> Signal SAVED as Good");
        isRecording = false;
      } else if (incomingByte == badSignal) {
        Serial.println("-> Signal DISCARDED as Bad");
        isRecording = false;
      }
    }
  }

  // 2. Lògica de gravació (mentre isRecording sigui true)
  if (isRecording) {
    int sensorValue = analogRead(emgPin);
    Serial.print(millis());
    Serial.print(",");
    Serial.print(sensorValue);
    Serial.print(",");
    Serial.println(label);

    delay(delayTimer); 
    
    // Opcional: Aturar automàticament després de X segons
    timer += delayTimer;
    if (timer >= (signalTime * 1000)) {
       Serial.println("Time out! Is it (g)ood or (b)ad?");
       // Aquí podríem forçar el final o esperar la tecla
    }
  }
}
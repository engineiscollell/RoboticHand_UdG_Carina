const int emg1Pin = 34; //Bíceps
const int emg2Pin = 35; //Tríceps

bool lecturaActiva = true;

const int llindarBiceps = 600; //600 per exemple, fa falta adapatar-ho a la potència dels miòcits de la Carina.
const int llindarTriceps = 550;

const int lockOutTime = 500; //ms

char posicioActual = 'R'; // R repós, T trípode, G grasp

void setup() {
  Serial.begin(115200);
  analogReadResolution(12); // ESP32
}
int v1 = 0;
int v2 = 0;

void loop() {
  // Lectura real sensors
  int v1 = analogRead(emg1Pin); //Bíceps
  int v2 = analogRead(emg2Pin); //Tríceps
  
  char tecla = Serial.read(); // Llegim la tecla premuda
  
  //Simulació pic, NO ESTÀ ACABAT, ENCARA HI HAN ERRORS.
  /*
  if(tecla == 'w'){
    if(v1 >= 700) v1-=700;
    else v1+=700;
    tecla = 'x';
  }
  if(tecla == 'e'){
    if(v2 >= 700) v2-=700;
    else v2+=700;
    tecla = 'x';
  }
  */
  
  // Start/stop
  if (tecla == ' ') { // Si la tecla és un espai
    lecturaActiva = !lecturaActiva;
  }

  if(v1 >= llindarBiceps){
    if(posicioActual == 'T') posicioActual = 'R';
    else posicioActual = 'T';
    delay(lockOutTime);
  }
  else if(v2 >= llindarTriceps){
    if(posicioActual == 'G') posicioActual = 'R';
    else posicioActual = 'G';
    delay(lockOutTime);
  }
  else posicioActual = 'R';

  // Print serial monitor
  if(lecturaActiva){
    Serial.print("Senyal pin 34 (Bíceps): ");
    Serial.print(v1);
    Serial.print(", Senyal pin 35 (Tríceps): ");
    Serial.print(v2);
    Serial.print(". Posició actual: ");
    Serial.println(posicioActual);
  }

  delay(100);
}

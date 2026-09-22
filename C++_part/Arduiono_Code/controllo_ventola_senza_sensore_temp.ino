#define FAN_PWM_PIN 29
#define FAN_TACH_PIN 27

volatile unsigned long pulseCount = 0;
unsigned long lastMillis = 0;
int currentRPM = 0;
int currentPWM = 0;

// Funzione chiamata ad ogni impulso del tachimetro
void countPulse() {
  pulseCount++;
}

void setup() {
  pinMode(FAN_PWM_PIN, OUTPUT);
  pinMode(FAN_TACH_PIN, INPUT_PULLUP);
  
  // Frequenza PWM a 25 kHz per ventole a 4 pin
  analogWriteFreq(25000); 
  Serial.begin(115200);
  
  // Interrupt sul fronte di discesa del segnale tachimetro
  attachInterrupt(digitalPinToInterrupt(FAN_TACH_PIN), countPulse, FALLING);
}

void loop() {
  // 1. Leggi comandi dal Raspberry Pi
  if (Serial.available() > 0) {
    int incomingValue = Serial.parseInt();
    while (Serial.available() > 0) Serial.read(); // Pulisci il buffer

    if (incomingValue >= 0 && incomingValue <= 255) {
      currentPWM = incomingValue;
      analogWrite(FAN_PWM_PIN, currentPWM);
    }
  }

  // 2. Calcola gli RPM ogni 1000 ms (1 secondo)
  unsigned long currentMillis = millis();
  if (currentMillis - lastMillis >= 1000) {
    noInterrupts(); // Blocca brevemente gli interrupt per leggere in sicurezza
    unsigned long pulses = pulseCount;
    pulseCount = 0; // Resetta il contatore
    interrupts();   // Riattiva gli interrupt
    
    // La maggior parte delle ventole PC genera 2 impulsi per giro completo
    currentRPM = (pulses * 60) / 2; 
    
    // 3. Invia la risposta al Raspberry Pi nel formato: PWM,RPM
    Serial.print(currentPWM);
    Serial.print(",");
    Serial.println(currentRPM);
    
    lastMillis = currentMillis;
  }
}

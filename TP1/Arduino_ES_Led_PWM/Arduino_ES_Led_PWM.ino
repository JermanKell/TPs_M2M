// On établit le seuil à atteindre à 3/4 de la tension usuelle
int T0 = 550;

// On definit la pin de sortie potentiometre
int potPin = 23;
// On definit les pins des leds
int led1Pin = 4;
int led2Pin = 5;

// Valeur retournee par le potentiometre
int analogValue = 0;

void setup() {
  
  Serial.begin(9600);

  // On configure en sortie les Leds
  pinMode(led1Pin, OUTPUT);
  pinMode(led2Pin, OUTPUT);  

}

void loop() {
  analogValue = analogRead(potPin);

  // Clignotement de la led 1 en fonction du seuil
  if (analogValue >= T0) { digitalWrite(led1Pin, HIGH); }
  else { digitalWrite(led1Pin, LOW); }

  // On gere l'intensite lumineuse en f(x) de la sortie du potentiometre
  analogWrite(led2Pin, analogValue);

  // Definition d'un temps de pause
  delay(100);
}

// On configure la PIN sur laquelle sera branchée la LED
#define LED1_PIN 13

void setup() {
  // On configure la vitesse de la communication série avec le MC
  //  9600 bauds ~= 9600 bits/sec
  Serial.begin(9600);
  
  // On definit la pin en sortie
  pinMode(LED1_PIN, OUTPUT);
  // On force l'extinction de la led en mettant l'etat de fonctionnement à BAS
  digitalWrite(LED1_PIN, LOW);

  // Affichage du msg dans le moniteur serie
  Serial.println("Lancement de l'application");

}

void loop() {
  // LED allumee
  digitalWrite(LED1_PIN, HIGH);
  // Affichage msg dans le moniteur serie
  Serial.println("Led allumee");
  // Attente d'une seconde
  delay(1000);
  // LED eteinte
  digitalWrite(LED1_PIN, LOW);
  // Affichage du msg
  Serial.println("Led eteinte");
  // Attente de 800 ms
  delay(800);

}

#include "DHT.h"
#include <Wire.h>
#include "rgb_lcd.h"
#include <ChainableLED.h>

// Definition de la broche de connection du capteur
#define pinSensor 2
// Definition du type de capteur utilise
#define DHTType DHT22

// Definition de la broche de sortie de la LED
#define NUM_LEDS 1

// Mode d'affichage: => true: afficheur LCD, serie sinon
boolean printMode = true;

// Initialisation du capteur
DHT dht(pinSensor, DHTType);

// Config. de la led
ChainableLED led(8, 9, NUM_LEDS);

// Declaration de l'utilisation d'un afficheur
rgb_lcd lcd;

// Conf des couleurs de l'afficheur
const int colorR = 255;
const int colorG = 0;
const int colorB = 0;

void setup() {
  // Initialisation de la communication serie
  Serial.begin(9600);

  dht.begin();
  Wire.begin();

  // Conf de l'afficheur
  if (printMode) {
    // Init nb colonnes et lignes
    lcd.begin(16, 2);
    
  }

}

void loop() {
  // Attente de la recuperation des donnees capteurs
  //  (laisser le temps pour l'acquisition)
  delay(2000);

  // Acquisition de l'humidite
  float humidity = dht.readHumidity();
  // Acquisition de la temperature en °C
  float tmpt = dht.readTemperature();

  // Verifie si les les infos recup sont corrompues
  if (isnan(humidity) || isnan(tmpt)) {
    if (printMode) {
      lcd.setCursor(0, 0);
      lcd.print("Pb de lecture de");
      lcd.setCursor(0, 1);
      lcd.print("donnees du capteur");
    }
    else {
      Serial.println("Pb de lecture des donnees capteur");
    }
    return;
  }

  if (printMode) {
    // On efface l'affichage
    lcd.clear();

    // On se positionne sur la ligne 1
    lcd.setCursor(0, 0);
    lcd.print("Humid.:");
    lcd.setCursor(10, 0);
    lcd.print(humidity);
    lcd.setCursor(14, 0);
    lcd.print(" %");
    
    lcd.setCursor(0, 1);
    lcd.print("Tmpture:   ");
    lcd.setCursor(10, 1);
    lcd.print(tmpt);
    lcd.setCursor(14, 1);
    lcd.print(" C");
  }
  else {
    Serial.print("Humidity: ");
    Serial.println(humidity);
    Serial.print("Temperature:  ");
    Serial.println(tmpt);
    
  }
  
  if (tmpt < 26.00) {
    led.setColorRGB(0, 0, 255, 0);
  }
  else {
    led.setColorRGB(0, 255, 0, 0);
  }

  delay(250);
}

#include <DHT.h>
#include <Wire.h>
#include "rgb_lcd.h"


// Definition de la broche de connection du capteur
#define pinSensor 5
// Definition du type de capteur utilise
#define DHTType DHT22

// Mode d'affichage: => true: afficheur LCD, serie sinon
boolean printMode = false;

// Initialisation du capteur
DHT dht(pinSensor, DHTType);

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

  // Conf de l'afficheur
  if (printMode) {
    // Init nb colonnes et lignes
    lcd.begin(16, 2);
    // Init background afficheur
    lcd.setRGB(colorR, colorG, colorB);
    
  }

}

void loop() {
  // Attente de la recuperation des donnees capteurs
  //  (laisser le temps pour l'acquisition)
  delay(1000);

  // Acquisition de l'humidite
  float humidity = dht.readHumidity();
  // Acquisition de la temperature en °C
  float tmpt = dht.readTemperature();

  // Verifie si les les infos recup sont corrompues
  if (isnan(humidity) || isnan(tmpt)) {
    if (printMode) {
      lcd.setCursor(1, 0);
      lcd.print("Pb de lecture de");
      lcd.setCursor(2, 0);
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
    lcd.setCursor(1, 0);
    lcd.print("Humidity:   %f", humidity);
    lcd.setCursor(2, 0);
    lcd.print("Tmpture:    %f°C", tmpt);
  }
  else {
    Serial.println("Humidity: %f", humidity);
    Serial.println("Temperature:    %.2f °C", tmpt);
  }
}

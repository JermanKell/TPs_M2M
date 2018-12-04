
// Environnement Arduino 1.0.6
// Mise en oeuvre d'un capteur de pression bme280
// Communication sur bus I2C

#include <Wire.h>
#include "rgb_lcd.h"
#include <ChainableLED.h>
#include "Seeed_BME280.h"

// Config de l'ecran LCD
rgb_lcd lcd;

// Nb de LEDs
#define NUM_LEDS 1

// Config de la LED
ChainableLED led(6, 7, NUM_LEDS);

//Config capter bme280
BME280 bme280;

void setup(void)
{ 
  // Configuration de la communication serie
  Serial.begin(9600);

  // Initialisation de l'afficheur LCD
  lcd.begin(16, 2);
  // On initialise une couleur de fond
  lcd.setRGB(255, 0, 0);

  if(!bme280.init()){
    Serial.println("Device error!");
  }

}

void loop (void)
{
  float temperature, pression, altitude;
  unsigned long up;
  
  delay(300);

  // Temperature apres compensation
  temperature = bme280.getTemperature();

  // Pression apres compensation
  pression = bme280.getPressure();

  // On recupere l'altitude
  altitude = bme280.calcAltitude(pression); 

  lcd.clear();

  // On positionne correctement le curseur pour ecrire
  lcd.setCursor(1, 0);
  lcd.print("Tmp: ");
  lcd.setCursor(5, 0);
  lcd.print(temperature);
  lcd.setCursor(11, 0);
  lcd.print("C");

  // On teste la valeur de temperaure recuperee
  // On affiche un message et on allume la LED en vert
  if (temperature < 26.00) {
    lcd.setRGB(0, 255, 0);
    lcd.setCursor(0, 1);
    lcd.print("Il fait bon ici!");
    led.setColorRGB(0, 0, 255, 0);
  }
  // Sinon on allume la LED en rouge avec un message ecrit sur l'afficheur
  else {
    lcd.setRGB(255, 0, 0);
    lcd.setCursor(2, 1);
    lcd.print("Trop chaud !");
    led.setColorRGB(0, 255, 0, 0);
  }
  
  delay(1000);
}

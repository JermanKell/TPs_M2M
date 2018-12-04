
// Environnement Arduino 1.0.6
// Mise en oeuvre d'un capteur de pression bme280
// Communication sur bus I2C

#include <Wire.h>
#include "rgb_lcd.h"
#include <ChainableLED.h>

// Capteur de pression : adressage I2C
// adresse sur le bus I2C du capteur bme280
#define Sensor_addr 0x76
#define Sensor_id 0xD0
#define Sensor_tmpt_reg 0xFA
#define Sensor_pres_reg 0xF7 
#define Sensor_hum_reg 0xFD
#define Sensor_ControlHumid 0xF2
#define Sensor_reg_Control 0xF4

#define BME280_REG_DIG_T1    0x88
#define BME280_REG_DIG_T2    0x8A
#define BME280_REG_DIG_T3    0x8C

#define BME280_REG_DIG_P1    0x8E
#define BME280_REG_DIG_P2    0x90
#define BME280_REG_DIG_P3    0x92
#define BME280_REG_DIG_P4    0x94
#define BME280_REG_DIG_P5    0x96
#define BME280_REG_DIG_P6    0x98
#define BME280_REG_DIG_P7    0x9A
#define BME280_REG_DIG_P8    0x9C
#define BME280_REG_DIG_P9    0x9E

#define BME280_REG_DIG_H1    0xA1
#define BME280_REG_DIG_H2    0xE1
#define BME280_REG_DIG_H3    0xE3
#define BME280_REG_DIG_H4    0xE4
#define BME280_REG_DIG_H5    0xE5
#define BME280_REG_DIG_H6    0xE7

uint16_t dig_T1;
int16_t dig_T2;
int16_t dig_T3;
uint16_t dig_P1;
int16_t dig_P2;
int16_t dig_P3;
int16_t dig_P4;
int16_t dig_P5;
int16_t dig_P6;
int16_t dig_P7;
int16_t dig_P8;
int16_t dig_P9;
uint8_t dig_H1;
int16_t dig_H2;
uint8_t dig_H3;
int16_t dig_H4;
int16_t dig_H5;
int8_t  dig_H6;
int32_t t_fine;

// Oversampling Setting pour bme280

// Coefficients de calibration du capteur de pression bme280 : déclaration

/*---------------------------------------------------------------------------------*/
// Fonctions de gestion du capteur de pression bme280

/*-----------------------------------------------*/
// Acquisition de la valeurs des parametres de calibration
void getbme280Calibration()
{
  dig_T1 = BME280Read16LE(BME280_REG_DIG_T1);
  dig_T2 = BME280ReadS16LE(BME280_REG_DIG_T2);
  dig_T3 = BME280ReadS16LE(BME280_REG_DIG_T3);

  dig_P1 = BME280Read16LE(BME280_REG_DIG_P1);
  dig_P2 = BME280ReadS16LE(BME280_REG_DIG_P2);
  dig_P3 = BME280ReadS16LE(BME280_REG_DIG_P3);
  dig_P4 = BME280ReadS16LE(BME280_REG_DIG_P4);
  dig_P5 = BME280ReadS16LE(BME280_REG_DIG_P5);
  dig_P6 = BME280ReadS16LE(BME280_REG_DIG_P6);
  dig_P7 = BME280ReadS16LE(BME280_REG_DIG_P7);
  dig_P8 = BME280ReadS16LE(BME280_REG_DIG_P8);
  dig_P9 = BME280ReadS16LE(BME280_REG_DIG_P9);

  dig_H1 = BME280Read8(BME280_REG_DIG_H1);
  dig_H2 = BME280Read16LE(BME280_REG_DIG_H2);
  dig_H3 = BME280Read8(BME280_REG_DIG_H3);
  dig_H4 = (BME280Read8(BME280_REG_DIG_H4) << 4) | (0x0F & BME280Read8(BME280_REG_DIG_H4 + 1));
  dig_H5 = (BME280Read8(BME280_REG_DIG_H5 + 1) << 4) | (0x0F & BME280Read8(BME280_REG_DIG_H5) >> 4);
  dig_H6 = (int8_t)BME280Read8(BME280_REG_DIG_H6);

  writeRegister(Sensor_ControlHumid, 0x05);  //Choose 16X oversampling
  writeRegister(Sensor_reg_Control, 0xB7);  //Choose 16X oversampling

}
/*-----------------------------------------------*/
// Calcul de la température connaissant ut.
// La valeur retournée est exprimée en 1/10 de °C (valeur entiére) puis convertie en °C (valeur réelle)
float bme280GetTemperature()
{
   float T = (t_fine * 5 + 128) >> 8;
  
  return T/100; 
}
/*-----------------------------------------------*/
// Calcul de la pression en hPa = 100 Pa
// Les paramètres de calibration doivent être connus
// Le calcul de la valeur du coefficient b5 doit être effectué au préalable.
// La valeur retournée est exprimé en Pa (valeur entiére), puis convertie en hPa (valeur réelle).

float bme280GetPressure(unsigned long up)
{
  
  return (float)up/256;
}

/*-----------------------------------------------*/
// Calcul de l'altitude par rapport au niveau de la mer
// A faire : compensation du calcul
//PARTIE OPTIONNELLE
float estime_altitude (float pression)
{
  float A = pression/101325;
  float B = 1/5.25588;
  float C = pow(A,B);
  C = 1.0 - C;
  C = C /0.0000225577;
  return C;
}

/*-----------------------------------------------*/
// Lecture d'un octet depuis le capteur bme280 
// à l'adresse 'address'
uint8_t BME280Read8(uint8_t address)
{
 Wire.beginTransmission(Sensor_addr);
 Wire.write(address);
 Wire.endTransmission();

 Wire.requestFrom(Sensor_addr, 1);
 while (Wire.available() < 1);

 return Wire.read();
}

/*-----------------------------------------------*/
// Lecture de 2 octets depuis le capteur bme280
// Premier octet : adresse 'address' (octet poids fort)
// Second octet : adresse 'address'+1 (octet poids faible)
uint16_t BME280Read16(uint8_t address)
{
  uint8_t msb, lsb;

  Wire.beginTransmission(Sensor_addr);
  Wire.write(address);
  Wire.endTransmission();

  Wire.requestFrom(Sensor_addr, 2);
  while (Wire.available() < 2);
  msb = Wire.read();
  lsb = Wire.read();
  
  return (uint16_t)msb << 8 | lsb;  
}

uint16_t BME280Read16LE(uint8_t address) {
  uint16_t data = BME280Read16(address);
  return (data >> 8) | (data << 8);  
}

int16_t BME280ReadS16(uint8_t address) {
  return (int16_t)BME280Read16(address);
}

int16_t BME280ReadS16LE(uint8_t address) {
  return (int16_t)BME280Read16LE(address);
}
/*-----------------------------------------------*/
// Lecture de 3 octets depuis le capteur bme280
uint32_t BME280Read24(uint8_t address) {
  uint32_t value = 0;

  Wire.beginTransmission(Sensor_addr);
  Wire.write(address);
  Wire.endTransmission();

  Wire.requestFrom(Sensor_addr, 3);
  while (Wire.available() < 3);
  value = Wire.read();
  value <<=8;
  value |= Wire.read();
  value <<=8;
  value |= Wire.read();

  return value;
}
/*-----------------------------------------------*/
// Lecture de la valeur non compensée de la température ut
void bme280ReadUT()
{
  int32_t var1, var2;

  int32_t adc_T = BME280Read24(Sensor_tmpt_reg);
  
  adc_T >>= 4;
  var1 = (((adc_T >> 3) - ((int32_t)(dig_T1 << 1))) *
    ((int32_t)dig_T2)) >> 11;

  var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) *
    ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) *
    ((int32_t)dig_T3)) >> 14;

  t_fine = var1 + var2;
}
/*-----------------------------------------------*/
// Lecture de la valeur non compensée de la pression up
long bme280ReadUP()
{
  int64_t var1, var2, p;
  long res;

  int32_t adc_P = BME280Read24(Sensor_pres_reg);
  adc_P >>= 4;

  var1 = ((int64_t)t_fine) - 128000;
  var2 = var1 * var1 * (int64_t)dig_P6;
  var2 = var2 + ((var1*(int64_t)dig_P5)<<17);
  var2 = var2 + (((int64_t)dig_P4)<<35);
  var1 = ((var1 * var1 * (int64_t)dig_P3)>>8) + ((var1 * (int64_t)dig_P2)<<12);
  var1 = (((((int64_t)1)<<47)+var1))*((int64_t)dig_P1)>>33;
  if (var1 == 0)
  {
    return 0; // avoid exception caused by division by zero
  }
  p = 1048576-adc_P;
  p = (((p<<31)-var2)*3125)/var1;
  var1 = (((int64_t)dig_P9) * (p>>13) * (p>>13)) >> 25;
  var2 = (((int64_t)dig_P8) * p) >> 19;
  p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7)<<4);

  res = (unsigned long)p;

  return res;
}

void writeRegister(uint8_t address, uint8_t val)
{
  Wire.beginTransmission(Sensor_addr); // start transmission to device
  Wire.write(address);       // send register address
  Wire.write(val);         // send value to write
  Wire.endTransmission();     // end transmission
}

/*---------------------------------------------------------------------------------*/
// Config de l'ecran LCD
rgb_lcd lcd;

// Nb de LEDs
#define NUM_LEDS 1

// Config de la LED
ChainableLED led(6, 7, NUM_LEDS);

void setup(void)
{ 
  Serial.begin(9600);

  lcd.begin(16, 2);
  lcd.setRGB(255, 0, 0);
  
  // Votre code à écrire
  Serial.println("***** Demarrage de l'application ****");
  
  // Initialidation bme280 : récupérer les paramètres de calibration du capteur
  uint8_t chip_id = 0;
  uint8_t retry = 0;

  while ((retry++ < 5) && (chip_id != 0x60)) {
    chip_id = BME280Read8(Sensor_id); 
    Serial.println(chip_id);
  }

  getbme280Calibration();    
}

void loop (void)
{
  float temperature, pression, altitude;
  unsigned long up;
  
  delay(300);

  // Calcule la temperature sans compensation a partir des valeurs des registres
  bme280ReadUT();
  // Temperature apres compensation
  temperature = bme280GetTemperature();

  // Calcul  de la pression sans compensation
  up = bme280ReadUP();
  // Pression apres compensation
  pression = bme280GetPressure(up);

  // On recupere l'altitude
  altitude = estime_altitude(pression); 

  lcd.clear();
  
  lcd.setCursor(1, 0);
  lcd.print("Tmp: ");
  lcd.setCursor(5, 0);
  lcd.print(temperature);
  lcd.setCursor(11, 0);
  lcd.print("C");
  
  if (temperature < 26.00) {
    lcd.setRGB(0, 255, 0);
    lcd.setCursor(0, 1);
    lcd.print("Il fait bon ici!");
    led.setColorRGB(0, 0, 255, 0);
  }
  else {
    lcd.setRGB(255, 0, 0);
    lcd.setCursor(2, 1);
    lcd.print("Trop chaud !");
    led.setColorRGB(0, 255, 0, 0);
  }
  
  delay(1000);
}

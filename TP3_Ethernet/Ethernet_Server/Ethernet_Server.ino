// Inclusion de toutes les bibliothèques necessaires
#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h>
#include <ChainableLED.h>

/**************************************************************************************************/
/***************************    Configuration capteur BME280    ***********************************/
/**************************************************************************************************/
// Capteur de pression : adressage I2C
// adresse sur le bus I2C du capteur BMP085
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

/*************************************************************************/

// Nb leds a brancher
#define NUM_LEDS 1

// Declaration @MAC du shield
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0E, 0xA7, 0x6F };

// @IP fixe
byte ip[] = { 192, 168, 1, 177 };
// Masque sous-reseau
byte subnet[] = { 255, 255, 255, 0 };

// Objet Ethernet serveur avec affectation d'un port d'ecoute
EthernetServer server(80);

// Objet Chainable pour le pilotage de LEDs
ChainableLED led(6, 7, NUM_LEDS);


/*********************************************************************/
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

// La valeur retournée est exprimée en 1/10 de °C (valeur entiére) puis convertie en °C (valeur réelle)
float bme280GetTemperature()
{
   float T = (t_fine * 5 + 128) >> 8;
  
  return T/100; 
}

// Retourne la valeur de la pression apres conversion en Pa
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
// Lecture d'un octet depuis le capteur BMP085 
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
// Lecture de 2 octets depuis le capteur BMP085
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

// Permet de lire le contenu d'un registre
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
// Lecture de 3 octets depuis le capteur BMP085
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

// Ecriture d'une valeur dans un registre
void writeRegister(uint8_t address, uint8_t val)
{
  Wire.beginTransmission(Sensor_addr); // start transmission to device
  Wire.write(address);       // send register address
  Wire.write(val);         // send value to write
  Wire.endTransmission();     // end transmission
}
/****************************************************************************************************/

void setup() {
  Serial.begin(9600);

  // Initialisation bus serie  
  Wire.begin();

   // Initialidation BMP085 : récupérer les paramètres de calibration du capteur
  uint8_t chip_id = 0;
  uint8_t retry = 0;

  // Au bout de 5 tentatives de recherche du BME280 sur le bus I2C
  // On cherche à le contacter grâce à l'adresse de son registre connu
  while ((retry++ < 5) && (chip_id != 0x60)) {
    chip_id = BME280Read8(Sensor_id); 
    Serial.println(chip_id);
  }

  getbme280Calibration(); 
  
  // On initialise les params reseau de la carte
  //  et on verifie le retour DHCP
  Ethernet.begin(mac, ip, subnet);
  
  // Initialisation serveur
  server.begin();
}

void loop() {

  // listen for incoming clients
  EthernetClient client = server.available();
  // On verifie si un client est connecte
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    String ledvalues = "";

    // Tant qu'un client est connecte, il faut executer une requete
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        ledvalues += c;

        // Affichage de la requete sur le moniteur serie de l'IDE
        Serial.print(c);

        // Lecture de la valeur de la temperature dans le registre
        bme280ReadUT();
        // Recuperation de la temperature formatee
        float tmpt = bme280GetTemperature();
        // Recuperation de la pression
        float pression = bme280GetPressure(bme280ReadUP());
        // Estimation de l'altitude en fonction de la pression calculee
        float altitude = estime_altitude(pression);

        // On verifie si la requete precedente retourne une ligne vide
        // Ce qui annonce le debut d'une seconde requete
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          
          client.println("<form action=\"\">");
          client.print("Temperature: ");       
          // Affichage de la temperature   
          client.print(tmpt);
          client.println(" C");
          client.println("<br>");

          client.print("Pression: ");
          // Affichage de la pression en hPa
          client.print(pression/100);
          client.println(" hPa");          
          client.println("<br>");

          client.print("Altitude: ");
          // Affichage de la valeur estimee de l'altitude a partir de la pression mesuree
          client.print(altitude);
          client.println(" m");          
          client.println("<br>");

          // Partie pour la modification de la luminance de la LED RGB
          client.println("<br>");
          client.println("Reglage LED RGB");
          client.println("<br>");
          
          client.println("<input type=\"number\" min='0' max='255' value='100' name=\"RedValue\" />");
          client.println("<input type=\"number\" min='0' max='255' value='100' name=\"GreenValue\" />");
          client.println("<input type=\"number\" min='0' max='255' value='100' name=\"BlueValue\" />");
          
          client.println("<br>");
          client.println("<input type=\"submit\" value=\"Submit\" />");
          client.println("</form>");
          
          client.println("</html>");

          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.print(ledvalues);

    // Recuperation des valeurs de couleurs depuis la requete effectuee depuis le client
    int redS = ledvalues.indexOf("RedValue");
    int greenS = ledvalues.indexOf("GreenValue");
    int blueS = ledvalues.indexOf("BlueValue");

    // On teste le retour de la methode valueOf
    if ((redS != -1) && (greenS != -1) && (blueS != -1)) {
      // On se positionne au niveau du caractere apres le '='
      int posR = redS + 9;
      int posG = greenS + 11;
      int posB = blueS + 10;

      // Recuperation d'une string contenant uniquement les infos de couleurs
      String red = ledvalues.substring(posR, greenS-1);
      String green = ledvalues.substring(posG, blueS-1);
      String blue = ledvalues.substring(posB, posB+3);

      // Pilotage LED RGB
      led.setColorRGB(0, red.toInt(), green.toInt(), blue.toInt());      
    }
    
    Serial.println("client disconnected");    
  }
  
}

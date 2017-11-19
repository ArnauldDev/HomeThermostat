/*
  HomeThermostat_v010.ino

  "Thermostat" example code.

  Matériels :
  - 1x carte Arduino Nano
  - 1x écran OLED 128x64 (contrôleur "SSD1306" en I2C)
  - 1x carte relais (1 contact normalement ouvert "NO")

  With universal 8bit Graphics Library, https://github.com/olikraus/u8glib/

  Copyright (c) 2012, olikraus@gmail.com
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification,
  are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright notice, this list
    of conditions and the following disclaimer.

    Redistributions in binary form must reproduce the above copyright notice, this
    list of conditions and the following disclaimer in the documentation and/or other
    materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/


#include "DHT.h"

#include "U8glib.h"

// Sortie de la commande du relais du thermostat :
#define DHT_PIN 2     // what digital pin we're connected to
#define DHT_TYPE DHT11   // DHT 11
DHT dht(DHT_PIN, DHT_TYPE);

int consigneOld = 20;
int consigneNew = 20;

//#define RelaisThermostatPin 12
#define RELAIS_PIN 12
#define RELAIS_OFF() {digitalWrite(RELAIS_PIN, HIGH);}
#define RELAIS_ON() {digitalWrite(RELAIS_PIN, LOW);}
// Sélectionner des sorties PWM pour la LED status RGB :
//#define LED_STATUS_RED_PIN   11 // LedStatusRedPin sortie PWM
//#define LED_STATUS_GREEN_PIN 12 // LedStatusGreenPin sortie PWM
//#define LED_STATUS_BLUE_PIN  13 // LedStatusBluePin sortie PWM
#define LED_STATUS_RED_PIN   13 // LedStatusRedPin sortie PWM

// setup u8g object, please remove comment from one of the following constructor calls
// IMPORTANT NOTE: The following list is incomplete. The complete list of supported
// devices with all constructor calls is here: https://github.com/olikraus/u8glib/wiki/device
//U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);	// I2C / TWI
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0 | U8G_I2C_OPT_NO_ACK | U8G_I2C_OPT_FAST);	// Fast I2C / TWI
//U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NO_ACK);	// Display which does not send AC
//U8GLIB_SSD1306_128X32 u8g(U8G_I2C_OPT_NONE);	// I2C / TWI

void draw(void) {
  // graphic commands to redraw the complete screen should be placed here
  //u8g.setFont(u8g_font_unifont);
  u8g.setFont(u8g_font_osb21);
  //  u8g.drawStr( 0, 22, "Hello World!");
}

void setup(void) {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);

  // initialize digital pin LED_STATUS as an output.
  //  pinMode(LED_STATUS_RED_PIN, OUTPUT);
  //  pinMode(LED_STATUS_GREEN_PIN, OUTPUT);
  pinMode(LED_STATUS_RED_PIN, OUTPUT);

  pinMode(RELAIS_PIN, OUTPUT);
  RELAIS_OFF();

  // flip screen, if required
  // u8g.setRot180();

  // assign default color value
  if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {
    u8g.setColorIndex(255);     // white
  }
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) {
    u8g.setColorIndex(3);         // max intensity
  }
  else if ( u8g.getMode() == U8G_MODE_BW ) {
    u8g.setColorIndex(1);         // pixel on
  }

  pinMode(8, OUTPUT);

  // Affichage sur l'ecran principale
  // picture loop
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_osb21);
    u8g.drawStr( 28, 30, "Home");
    u8g.setFont(u8g_font_osb18);
    u8g.drawStr( 0, 56, "Thermostat");
  } while ( u8g.nextPage() );
  //    u8g.drawStr( 0, 16, "Consigne");
  //    u8g.drawStr( 0, 37, "Temperature");
  delay(5000);

  dht.begin();

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");
  Serial.print("Heat index: ");
  Serial.print(hic);
  Serial.print(" *C ");
  Serial.print(hif);
  Serial.println(" *F");
}

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;        // will store last time LED was updated

// constants won't change :
const long interval = 3000;           // interval at which to blink (milliseconds)

void loop(void) {
  // Lecture de la consigne
  // read the input on analog pin 0:
  int consigneValue = analogRead(A0);
  consigneNew = map(consigneValue, 0, 1023, 15, 30);
  
  while (consigneNew != consigneOld) {
    consigneOld = consigneNew;
    // Affichage sur l'ecran principale
    char charBuf[10];               //temporarily holds data from vals
    dtostrf(consigneNew, 2, 1, charBuf);
    // picture loop
    u8g.firstPage();
    do {
      u8g.setFont(u8g_font_osb18);
      u8g.drawStr( 5, 24, "Consigne :");
      u8g.drawStr( 22, 57, charBuf);
      u8g.drawStr( 82, 57, "C");
    } while ( u8g.nextPage() );
    delay(1000);
    consigneValue = analogRead(A0);
    consigneNew = map(consigneValue, 0, 1023, 15, 30);
  }

  // print out the value you read:
//  Serial.println(consigneNew);

  digitalWrite(LED_STATUS_RED_PIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(100);
  digitalWrite(LED_STATUS_RED_PIN, LOW);    // turn the LED off by making the voltage LOW

  // Mesure de la température ambiante
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Compute heat index in Celsius (isFahreheit = false)
  float mesureTemp = dht.computeHeatIndex(t, h, false);

  // Vérification de la demande de chauffage
  //
  if (mesureTemp < consigneNew) {
    //digitalWrite(LED_STATUS_RED_PIN, HIGH);
    //digitalWrite(RELAIS_PIN, LOW);
    RELAIS_ON();
  } else {
    //digitalWrite(LED_STATUS_RED_PIN, LOW);
    //digitalWrite(RELAIS_PIN, HIGH);
    RELAIS_OFF();
  }

  // check to see if it's time to blink the LED; that is, if the
  // difference between the current time and last time you blinked
  // the LED is bigger than the interval at which you want to
  // blink the LED.
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    // Affichage de la température sur l'ecran principale
    char charBuf[10];               //temporarily holds data from vals
    dtostrf(mesureTemp, 2, 1, charBuf);
    // picture loop
    u8g.firstPage();
    do {
      //    u8g.setFont(u8g_font_unifont);
      //    u8g.drawStr( 0, 10, "Temperature");
      u8g.setFont(u8g_font_osb21);
      u8g.drawStr( 23, 45, charBuf);
      u8g.drawStr( 87, 45, "C");
    } while ( u8g.nextPage() );
  }

  // rebuild the picture after some delay
  //  delay(3000);
  //  delay(1);        // delay in between reads for stability
}


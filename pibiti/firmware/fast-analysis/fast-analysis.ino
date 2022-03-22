/*Código para envio dos dados via MQTT*/

#include <Arduino.h>
#include <ESP8266WiFiMulti.h>
//#include <Wire.h>
//#include <EEPROM.h>
//#include <ArduinoJson.h>

//#include "ESP8266httpUpdate.h"

//#include "Adafruit_MQTT.h"
//#include "Adafruit_MQTT_Client.h"
#include <PubSubClient.h>

#include "sensor.h"
#include "secure.hpp"

//------------------------------------------------------------------------

//#define Version "2.0.0.0"
//#define MakeFirmwareInfo(k, v) "&_FirmwareInfo&k=" k "&v=" v "&FirmwareInfo_&"

//------------------------------------------------------------------------

void callback(char *topic, byte *payload, unsigned int length)
{
  // handle message arrived
}

//------------------------------------------------------------------------

/*************************** REGISTRADORES DO SENSOR ***************************************/

const uint8_t MPU_ADDR = 0x68;
const uint8_t WHO_AM_I = 0x75;
const uint8_t PWR_MGMT_1 = 0x6B;
const uint8_t GYRO_CONFIG = 0x1B;  // Registrador que configura a escala do giroscópio.
const uint8_t ACCEL_CONFIG = 0x1C; // Registrador que configura a escala do acelerômetro.
const uint8_t ACCEL_XOUT = 0x3B;   //
const uint8_t GYRO_SCALE = 8;      // Escala do giroscópio
const uint8_t ACCEL_SCALE = 8;     // Escala do acelerômetro

//------------------------------------------------------------------------
//  Buffers

int16_t zeros[6];  // Média dos dados da calibração. Utilizados como zero.
int16_t buff[6];   // Dado atual dos sensores acc e gyr no tipo raw (puro).

//------------------------------------------------------------------------
//  Variaveis diversas

int16_t gravityRAW;     // Gravidade em raw (varia de acordo com o range do acelerometro escolhido); pode ser substituído por uma simples formula, mas eu preferi assim


//------------------------------------------------------------------------
//  Constantes diversas

//const float localGravity = 9.7803;
const float halfRange = 32768; // Metade do range de 16 bits


//------------------------------------------------------------------------

/*
 * DEFINIÇÕES DE WI-FI
 */

WiFiClient wifiClient;
ESP8266WiFiMulti wifiMulti;

Sensor sensor;
MyESP esp;
PubSubClient client(IO_SERVER, IO_SERVERPORT, callback, wifiClient);

/*
 * DEFINIÇÕES DE TEMPORIZAÇÃO
 */

unsigned long prevCheckTime = 0;
unsigned long checkInterval = 1000;

void setup()
{

  Serial.begin(500000);
  Serial.println("\n\nANALISE RÁPIDA DE VIBRAÇÃO");

  //wifiSet(wifiMulti);
  esp.setWifi(wifiMulti);
  
  sensor.wakeup();
  //sensor.setRange(localGravity); //-->
  gravityRAW = halfRange / 4;
  sensor.calibrate(0);
  
}

void loop()
{
  unsigned long currTime = millis();

  if (currTime - prevCheckTime >= checkInterval)
  {
    prevCheckTime = currTime;
  }

  //  Adafruit_MQTT_Subscribe *subscription;
  //  while ((subscription = mqtt.readSubscription(10))) {
  //    // ROTINA DE  RECEBIMENTO DE DADOS
  //    /*
  //        if (subscription == &onoffbutton) {
  //          message = (char *)onoffbutton.lastread;
  //          Serial.print("message: ");
  //          Serial.println(message);
  //        }
  //    */
  //    if (subscription == &doCapture) {
  //      message = (char *)updateButton.lastread;
  //
  //      if (strcmp(message, "begin") == 0)
  //        captureData();
  //        sendData();
  //    }
  //  }
}

void captureData()
{
}

void sendData()
{
}

/*
void wifiSet(ESP8266WiFiMulti wifiMulti)
{
  WiFi.mode(WIFI_STA);

  wifiMulti.addAP(WLAN_SSID, WLAN_PASS);

  Serial.println("Conectando à rede Wi-Fi.");
  for (int retry = 15; (retry >= 0 && wifiMulti.run() != WL_CONNECTED); retry--)
  {
    if (retry == 0)
      while (1)
        ;
    Serial.print(".");
    delay(1000);
  }

  Serial.print("\nWi-Fi conectada. IP ");
  Serial.println(WiFi.localIP());
}
*/

// void mqttConnect() {
//   Serial.print("Conectando ao servidor MQTT... ");
//
//   for (int retry = 5; (retry >= 0 && mqtt.connect() != 0); retry--) {
//     if (retry == 0)
//       while (1)
//         ;
//
//     Serial.println("Erro. Nova conexão em 5 segundos...");
//     mqtt.disconnect();
//     delay(5000);
//   }
//   Serial.println("Broker MQTT conectado!");
// }

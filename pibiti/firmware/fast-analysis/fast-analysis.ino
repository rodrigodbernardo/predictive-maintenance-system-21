#include <Arduino.h>
#include <ESP8266WiFiMulti.h>
//#include <Wire.h>
//#include <EEPROM.h>
//#include <ArduinoJson.h>

//#include "ESP8266httpUpdate.h"

#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#include "secure.hpp"

//#define Version "2.0.0.0"
//#define MakeFirmwareInfo(k, v) "&_FirmwareInfo&k=" k "&v=" v "&FirmwareInfo_&"

/*
 * DEFINIÇÕES DE WI-FI
 */

WiFiClient wifiClient;
ESP8266WiFiMulti wifiMulti;

/*
 * DEFINIÇÕES DE TEMPORIZAÇÃO 
 */

unsigned long prevCheckTime = 0;
unsigned long checkInterval = 1000;


void setup(){
Serial.begin(500000);
Serial.println("\n\nANALISE RÁPIDA DE VIBRAÇÃO");

wifiSet(wifiMulti);


}

void loop(){
  unsigned long currTime = millis();
  
  if (currTime - prevCheckTime >= checkInterval) {
    prevCheckTime = currTime;


  }

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(10))) {
    // ROTINA DE  RECEBIMENTO DE DADOS
    /*
        if (subscription == &onoffbutton) {
          message = (char *)onoffbutton.lastread;
          Serial.print("message: ");
          Serial.println(message);
        }
    */
    if (subscription == &doCapture) {
      message = (char *)updateButton.lastread;

      if (strcmp(message, "begin") == 0)
        captureData();
        sendData();
    }
  }
}

void captureData(){

}

void sendData(){

}

void wifiSet(ESP8266WiFiMulti wifiMulti) {
  WiFi.mode(WIFI_STA);

  wifiMulti.addAP(WLAN_SSID, WLAN_PASS);

  Serial.println("Conectando à rede Wi-Fi.");
  for (int retry = 15; (retry >= 0 && wifiMulti.run() != WL_CONNECTED); retry--) {
    if (retry == 0)
      while (1)
        ;
    Serial.print(".");
    delay(1000);
  }

  Serial.print("\nWi-Fi conectada. IP ");
  Serial.println(WiFi.localIP());
}

void mqttConnect() {
  Serial.print("Conectando ao servidor MQTT... ");

  for (int retry = 5; (retry >= 0 && mqtt.connect() != 0); retry--) {
    if (retry == 0)
      while (1)
        ;

    Serial.println("Erro. Nova conexão em 5 segundos...");
    mqtt.disconnect();
    delay(5000);
  }
  Serial.println("Broker MQTT conectado!");
}
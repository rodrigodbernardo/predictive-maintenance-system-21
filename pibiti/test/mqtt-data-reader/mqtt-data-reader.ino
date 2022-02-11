//
//#include <Arduino.h>
//#include <ESP8266WiFiMulti.h>
//#include <ArduinoJson.h>
//
//#include "Adafruit_MQTT.h"
//#include "Adafruit_MQTT_Client.h"
//
//#include "secure.hpp"
//
///*************************** VARIAVEIS ************************************/
//
//char *message;
//
///*************************** CONSTANTES              ***************************************/
//unsigned long prevCaptureTime = 0;
//unsigned long pingInterval = 1000;
//
//
///*************************** Pinos E/S                ***************************************/
//
//
//
///************ OBJETOS ******************/
//
//WiFiClient wifiClient;
//ESP8266WiFiMulti wifiMulti;
//Adafruit_MQTT_Client mqtt(&wifiClient, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
//
///****************************** TÓPICOS MQTT ***************************************/
//
//// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
//
//Adafruit_MQTT_Publish sendData = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/send");
//Adafruit_MQTT_Subscribe receiveData = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/receive");
//
///*************************** CODIGO ************************************/
//
//void setup() {
//  Serial.begin(500000);
//
//  wifiSet(wifiMulti);
//  mqtt.subscribe(&receiveData);
//}
//
//void loop() {
//  if (!mqtt.connected())
//    mqttConnect();
//
//  unsigned long currTime = millis();
//
//  if (currTime - prevCaptureTime >= pingInterval) {
//    prevCaptureTime = currTime;
//     if(! mqtt.ping()) {
//     mqtt.disconnect();
//     }
//  }
//
//  Adafruit_MQTT_Subscribe *subscription;
//  while ((subscription = mqtt.readSubscription(10))) {
//
//    if (subscription == &receiveData) {
//      message = (char *)receiveData.lastread;
//      Serial.println(message);
//    }
//  }
//}
//
//void mqttConnect() {
//  Serial.print("Conectando ao servidor MQTT... ");
//
//  for (int retry = 5; (retry >= 0 && mqtt.connect() != 0); retry--) {
//    if (retry == 0)
//      while (1)
//        ;
//
//    Serial.println("Erro. Nova conexão em 5 segundos...");
//    mqtt.disconnect();
//    delay(5000);
//  }
//  Serial.println("Broker MQTT conectado!");
//}
//
//void wifiSet(ESP8266WiFiMulti wifiMulti) {
//  WiFi.mode(WIFI_STA);
//
//  wifiMulti.addAP(WLAN_SSID, WLAN_PASS);
//
//  Serial.println("Conectando à rede Wi-Fi.");
//  for (int retry = 15; (retry >= 0 && wifiMulti.run() != WL_CONNECTED); retry--) {
//    if (retry == 0)
//      while (1)
//        ;
//    Serial.print(".");
//    delay(1000);
//  }
//
//  Serial.print("\nWi-Fi conectada. IP ");
//  Serial.println(WiFi.localIP());
//}
//


#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include "secure.hpp"

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived :");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

WiFiClient wifiClient;
PubSubClient client(AIO_SERVER, AIO_SERVERPORT, callback, wifiClient);

void setup() {
  Serial.begin(500000);
  setup_wifi();
  setup_mqtt();
  client.setServer(AIO_SERVER, 1883);
  client.setCallback(callback);
}

void loop() {
  // put your main code here, to run repeatedly:
  client.loop();
//  if (client.connected()) {
//    client.publish("outTopic", "hello world");
//    Serial.println("Enviando dado");
//  } else {
//    setup_mqtt();
//  }

  delay(2000);
}

void setup_wifi() {


  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WLAN_SSID, WLAN_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup_mqtt() {

  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    //String clientId = "ESP8266Client-";
    //clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect("cliente", AIO_USERNAME, AIO_KEY)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }

}

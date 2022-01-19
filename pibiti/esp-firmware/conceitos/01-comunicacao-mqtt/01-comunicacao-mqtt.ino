#include <ESP8266WiFiMulti.h>
#include <PubSubClient.h>
//#include <ArduinoJson.h>

#include "secure.hpp"

using namespace std;

String mensagem_entrada;
int num = 0;
int volta = 0;

ESP8266WiFiMulti wifiMulti;

WiFiClient wifiClient;
PubSubClient MQTT(wifiClient);

void setup()
{
    Serial.begin(115200);

    MQTT.setServer(broker_addr, broker_port);
    MQTT.setCallback(inputMQTT);

    setupWiFi(wifiMulti);
}

void loop()
{
    if (!MQTT.connected())
    {
        setupMQTT();
    }

    MQTT.loop();

    // pequena rotina para simular o dado a ser enviado.

    if (num == 10)
        volta = 1;

    if (num == 0)
        volta = 0;

    if (volta == 0)
        num++;
    else
        num--;
    
    char num_[20];
    snprintf(num_, 20,"%i", num);
    MQTT.publish(topico_teste_saida, num_);
    //Serial.println(num);
    delay(1000);
}

// Funções

void setupMQTT()
{

    String deviceID = "ESP8266Client-";
    //deviceID += String(random(0xffff), HEX);
    deviceID += WiFi.macAddress();

    Serial.println("Tentando conectar ao broker como " + deviceID);
    if (MQTT.connect(deviceID.c_str()))
    {
        Serial.println("\nBroker conectado!");
        MQTT.subscribe(topico_teste_entrada);
    }
    else
    {
        Serial.println("Erro. Tentando novamente em 5s.");
        delay(5000);
    }
}

void setupWiFi(ESP8266WiFiMulti wifiMulti)
{
    while (wifiMulti.run() != WL_CONNECTED)
    {
        WiFi.mode(WIFI_STA);
        wifiMulti.addAP(SSID_01, PASS_01);

        Serial.println("Tentando conectar à rede Wi-Fi.");
        delay(500);
    }

    Serial.print("\nWi-Fi conectada. IP ");
    Serial.println(WiFi.localIP());
}

void inputMQTT(char *topic, uint8_t *payload, unsigned int length)
{
  mensagem_entrada = "";
  for (int i = 0; i < length; i++)
  {
    char c = (char)payload[i];
    mensagem_entrada += c;
  }

  Serial.println("Input: " + mensagem_entrada);

  switch(mensagem_entrada){
      case "liga1":
          digitalWrite(D1, HIGH);
          break;
      case "liga2":
          digitalWrite(D2, HIGH);
          break;
   
          
  }
    
}

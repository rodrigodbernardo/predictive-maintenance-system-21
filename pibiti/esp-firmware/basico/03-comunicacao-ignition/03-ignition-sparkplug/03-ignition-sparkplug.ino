#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <Wire.h>

#include "mpu-pibiti.h"
#include "secure.hpp"

//=======================

/**Constantes**/

/***Configuração do sensor***/

const uint8_t MPU_ADDR = 0x68;
const uint8_t WHO_AM_I = 0x75;
const uint8_t PWR_MGMT_1 = 0x6B;
const uint8_t GYRO_CONFIG = 0x1B;  // Registrador que configura a escala do giroscópio.
const uint8_t ACCEL_CONFIG = 0x1C; // Registrador que configura a escala do acelerômetro.
const uint8_t ACCEL_XOUT = 0x3B;   //
const uint8_t GYRO_SCALE = 8;      // Escala do giroscópio
const uint8_t ACCEL_SCALE = 16;    // Escala do acelerômetro

/***Constantes adicionais***/

#define sda D6
#define scl D5

const float halfRange = 32768; // Metade do range de 16 bits
const long interval = 1000;

/**Variaveis**/

/***Buffers***/

int16_t zeros[6];  // Média dos dados da calibração. Utilizados como zero.
int16_t buff[6];   // Dado atual dos sensores acc e gyr no tipo raw (puro).
int16_t buff_temp; // Dado atual do sensor tmp no tipo raw (puro).
float buff_[6];    // Dado atual dos sensores acc e gyr em unidade do SI (m/s^2 e grau/s).
float buff_temp_;  // Dado atual do sensor tmp em unidade do SI (ºC).

/***Variaveis adicionais***/

int16_t gravityRAW;     // Gravidade em raw (varia de acordo com o range do acelerometro escolhido); pode ser substituído por uma simples formula, mas eu preferi assim
float range_a, range_g; // Meio range dos sensores acc e gyr, respectivamente
unsigned long previousMillis = 0;

//=======================

void callback(char *topic, byte *payload, unsigned int length)
{
  // handle message arrived
}

sensor mpu;
WiFiClient wifiClient;
ESP8266WiFiMulti wifiMulti;
PubSubClient client(broker_addr, broker_port, callback, wifiClient);
//StaticJsonDocument<100> data;

void setup()
{
  Serial.begin(500000);
  Wire.begin(sda, scl);

  delay(3000);

  mpu.setWifi(wifiMulti);
  //mpu.setMqtt(MQTT);
  mpu.wakeUp();
  mpu.setRange(9.7803); // Envia o valor da gravidade no local
  mpu.calibrate(0);     // Calibra o sensor com base na orientação atual do sistema. Por padrão, considera o eixo X como apontando para cima/baixo
}

void loop()
{
  client.loop();
  unsigned long currentMillis = millis();



  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    if (client.connected()) {  // Verifica se o broker está desconectado

      mpu.read(0);      // Faz uma leitura no sensor. Quando recebe 0 como argumento, realiza um no sensor com base na calibração
      mpu.print(0);     // Printa a ultima captura na Serial. Caso o argumento seja 0, printa o valor bruto. Se for 1, printa o valor no SI
      mpu.send(client,0); // Envia a ultima captura por MQTT.
    }
    else
      setBroker(); // Conecta ao broker.
  }
}

void setBroker() //setupMQTT
{

  String deviceID = "ESP-";
  //deviceID += String(random(0xffff), HEX);
  deviceID += WiFi.macAddress();

  Serial.println("Conectando. ID: " + deviceID);
  if (client.connect("cliente", "cliente", "cliente"))
  {
    Serial.println("\nBroker ok!");
    client.subscribe(topico_teste_entrada);
  }
  else
  {
    Serial.println("Erro. Tentando novamente.");
    delay(2000);
  }
}

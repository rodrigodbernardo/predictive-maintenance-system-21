#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
//
#include "mpu-pibiti.h"
//#include "key.h"

#define sda D6
#define scl D5

/**Constantes**/
/***Configuração do sensor***/
const uint8_t MPU_ADDR = 0x68;
const uint8_t WHO_AM_I = 0x75;
const uint8_t PWR_MGMT_1 = 0x6B;
const uint8_t GYRO_CONFIG = 0x1B;  //REGISTRADOR RESPONSAVEL POR CONFIGURAR A ESCALA DO GIROSCOPIO
const uint8_t ACCEL_CONFIG = 0x1C; //REGISTRADOR RESPONSAVEL POR CONFIGURAR A ESCALA DO ACELEROMETRO
const uint8_t ACCEL_XOUT = 0x3B;
const uint8_t GYRO_SCALE = 0b00001000; //-> ver documentação para saber a escala de conversao do sensor
const uint8_t ACCEL_SCALE = 0b00001000;

/***Constantes físicas***/
const float gravity = 9.7803;       //--> Aceleração gravitacional estimada em Fortaleza
const float halfRange = 32768;      //--> Metade do range de 16 bits

/**Variaveis**/
/***Buffers***/

int16_t zeros[6];                   //--> Guarda a media dos dados da calibração. Utilizados como zero.

int16_t buff [6];                   //--> Guarda os ultimos dados dos sensores acc e gyr no tipo raw (puro).
float   buff_[6];                   //--> Guarda os ultimos dados dos sensores acc e gyr em unidade do SI (m/s^2 e grau/s).
int16_t buff_temp;                  //--> Guarda os ultimos dados do sensor tmp no tipo raw (puro).
float   buff_temp_;                 //--> Guarda os ultimos dados do sensor tmp em unidade do SI (ºC).

int16_t gravityConst;               //--> Especifica quanto equivale a variavel 'gravity' no tipo raw (varia de acordo com o range do acelerometro escolhido); pode ser substituído por uma simples formula, mas eu preferi assim

bool    rawFlag = 1;                //--> Especifica se os dados serao mostrado no tipo RAW ou SI. 1 para SI; 0 para RAW

float   range_a, range_g;           //--> Guarda o range dos sensores acc e gyr, respectivamente


/**Objetos**/

ESP8266WiFiMulti wifiMulti;
sensor mpu;

WiFiClient wifiClient;
PubSubClient MQTT(wifiClient);
StaticJsonDocument<100> data;
/**Rotinas**/

void setup() {
  Serial.begin(500000);
  Wire.begin(sda, scl);

  mpu.setWifi(wifiMulti);
  mpu.setMqtt(MQTT);
  delay(3000);

  mpu.wakeup();
  mpu.setRange();
  mpu.calibrate();
}

void loop() {

  mpu.read(0);
  mpu.send(MQTT);
  mpu.print();

  if (!MQTT.connected())
    brokerSetup();

  MQTT.loop();

  delay(100);
}

void brokerSetup()//setupMQTT
{

  String deviceID = "ESP8266Client-";
  //deviceID += String(random(0xffff), HEX);
  deviceID += WiFi.macAddress();

  Serial.println("Trying to connect to MQTT Broker as " + deviceID);
  if (MQTT.connect(deviceID.c_str()))
  {
    Serial.println("\nBroker connected!");
    MQTT.subscribe("rdba/inTopic");
  }
  else
  {
    Serial.println("Error. Trying again in 5 seconds.");
    delay(5000);
  }
}
/*
  void wifiStart() {
  wifiMulti.addAP("FLAVIO_02", "8861854611");

  WiFi.mode(WIFI_STA);

  while (wifiMulti.run() != WL_CONNECTED)
  {
    delay(100);
    Serial.print(".");
  }

  Serial.println("\n\nCONNECTED TO " + WiFi.SSID());
  Serial.print("IP ADDRESS: ");
  Serial.println(WiFi.localIP());
  }
*/

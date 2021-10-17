/*

DESCRIÇÃO:

Todo o programa é orientado a objetos.
Envio de dados do sensor através de MQTT

*/

#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESP8266WiFiMulti.h>
#include <Wire.h>

#include "mpu-pibiti.h"
#include "secure.key"

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
const uint8_t GYRO_SCALE = 8; //-> ver documentação para saber a escala de conversao do sensor
const uint8_t ACCEL_SCALE = 16;
/***Constantes físicas***/
//const float gravity = 9.7803;  //--> Aceleração gravitacional estimada em Fortaleza
const float halfRange = 32768; //--> Metade do range de 16 bits

/**Variaveis**/
/***Buffers***/

int16_t zeros[6]; //--> Guarda a media dos dados da calibração. Utilizados como zero.

int16_t buff[6];   //--> Guarda os ultimos dados dos sensores acc e gyr no tipo raw (puro).
float buff_[6];    //--> Guarda os ultimos dados dos sensores acc e gyr em unidade do SI (m/s^2 e grau/s).
int16_t buff_temp; //--> Guarda os ultimos dados do sensor tmp no tipo raw (puro).
float buff_temp_;  //--> Guarda os ultimos dados do sensor tmp em unidade do SI (ºC).

int16_t gravityRAW; //--> Especifica quanto equivale a gravidade em raw (varia de acordo com o range do acelerometro escolhido); pode ser substituído por uma simples formula, mas eu preferi assim

//bool rawFlag = 1; //--> Especifica se os dados serao mostrado no tipo RAW ou SI. 1 para SI; 0 para RAW

float range_a, range_g; //--> Guarda o range dos sensores acc e gyr, respectivamente

/**Objetos**/

ESP8266WiFiMulti wifiMulti;
sensor mpu;

WiFiClient wifiClient;
PubSubClient MQTT(wifiClient);
StaticJsonDocument<100> data;


/**Rotinas**/

void setup()
{
  Serial.begin(500000);
  Wire.begin(sda, scl);

  delay(3000);
  
  mpu.setWifi(wifiMulti);
  mpu.setMqtt(MQTT);

  delay(3000);

  mpu.wakeup();
  mpu.setRange(9.7803); // Envia o valor da gravidade no local
  mpu.calibrate(0);            // Calibra o sensor com base na orientação atual do sistema. Por padrão, considera o eixo X como apontando para cima/baixo
}

void loop()
{
  if (!MQTT.connected())      // Conecta ao broker, caso necessario.
    mpu.setBroker(MQTT);
    
  mpu.read(0);                // Faz uma leitura no sensor. Quando recebe 0 como argumento, realiza um no sensor com base na calibração
  mpu.print(1);               // Printa a ultima captura na Serial. Caso o argumento seja 0, printa o valor bruto. Se for 1, printa o valor no SI
  mpu.send(MQTT);             // Envia a ultima captura por MQTT.
  
  MQTT.loop();                // Verifica se alguma mensagem foi recebida via MQTT.
  delay(1000);
}

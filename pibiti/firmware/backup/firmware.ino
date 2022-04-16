#include <Wire.h>
#include <PubSubClient.h>
#include <ESP8266WiFiMulti.h>
#include <ArduinoJson.h>

#include "pibiti.h"
#include "secure.hpp"

//-------------------------------------

#define sda D6
#define scl D5

//-------------------------------------

const uint8_t MPU_ADDR = 0x68;
const uint8_t WHO_AM_I = 0x75;
const uint8_t PWR_MGMT_1 = 0x6B;
const uint8_t GYRO_CONFIG = 0x1B;  // Registrador que configura a escala do giroscópio.
const uint8_t ACCEL_CONFIG = 0x1C; // Registrador que configura a escala do acelerômetro.
const uint8_t ACCEL_XOUT = 0x3B;   //
const uint8_t GYRO_SCALE = 8;      // Escala do giroscópio
const uint8_t ACCEL_SCALE = 8;     // Escala do acelerômetro

//-------------------------------------

unsigned long prevCheckTime = 0;
unsigned long checkInterval = 100;

unsigned long currTime;

//-------------------------------------

int16_t buffer[7];
String names[7] = {"AcX:", ",AcY:", ",AcZ:", ",GyX:", ",GyY:", ",GyZ:", ",Tmp:"};

int16_t v_data[3000][7];

//-------------------------------------

int n_packets, sample_period, command;
long n_captures = 0;
long capt_counter = 0;
String sts;

//-------------------------------------

void callback(char *topic, byte *payload, unsigned int length)
{
  StaticJsonDocument<256> mqtt_input;
  deserializeJson(mqtt_input, payload);

  command = mqtt_input["cmd"];
  n_packets = mqtt_input["npk"];
  n_captures = mqtt_input["ncp"];
  sample_period = mqtt_input["spe"];

  sts  = "Comando recebido: ";
  sts += command ;
  sts += " -- ";
  sts += n_packets;
  sts += " pacotes -- ";
  sts += n_captures;
  sts += " capturas -- " ;
  sts += sample_period;
  sts += "ms ";



  Serial.println(sts);
}

//-------------------------------------

ESP8266WiFiMulti wifiMulti;

WiFiClient espClient;
PubSubClient mqtt(IO_SERVER, 1883, callback, espClient);

Sensor mpu;
MyESP esp;

//-------------------------------------

void setup()
{
  Serial.begin(2000000);
  delay(2000);
  Serial.println("\n\nAnálise de vibração");

  Wire.begin(sda, scl);
  mpu.wakeup();

  esp.setWifi(wifiMulti);
}

//-------------------------------------

void loop()
{
  /*
    while (command == 1)
    {
    for (int packet = 0; packet < n_packets; packet++)
    {
      while((command == 1) && (capt_counter < n_captures))
      {
        prevCheckTime = millis();


        mpu.read();
      }
      mqtt.loop();


      esp.sendData();
      Serial.println("Envio ok");
    }
    command = -1;
    }
  */

  if (command == 1)
  { // Analise em tempo real
    mqtt.publish(topico_saida, sts.c_str());
    for (int packet = 0; packet < n_packets; packet++)
    {
      prevCheckTime = millis();
      while ((command == 1) && (capt_counter < n_captures)) {

        currTime = millis();

        if ((currTime - prevCheckTime >= sample_period))
        {
          prevCheckTime = currTime;

          mpu.read();
          //esp.print();
          //esp.sendData();

          capt_counter++;
        }
        mqtt.loop();
      }

      capt_counter = 0;
      mqtt.publish(topico_saida, "fim");
    }
    command = -1;
  }

  if (command == 2)
  { // Analise em tempo real
    mqtt.publish(topico_saida, sts.c_str());

    prevCheckTime = millis();
    while ((command == 2) && (capt_counter < n_captures)) {
      currTime = millis();
      if ((currTime - prevCheckTime >= sample_period))
      {
        prevCheckTime = currTime;

        mpu.read();
        
        for (int axis = 0; axis < 7; axis++)
          v_data[capt_counter][axis] = buffer[axis];
          
        //esp.print();
        

        capt_counter++;
      }
      mqtt.loop();
    }
    esp.sendStoredData();
    command = -1;
    capt_counter = 0;
    mqtt.publish(topico_saida, "fim");
  }

  if (command == -1)
  {
    ;
  }

  if (wifiMulti.run() != WL_CONNECTED)
  {
    esp.setWifi(wifiMulti);
  }

  if (!mqtt.connected())
  {
    esp.setMqtt();
  }
  mqtt.loop();
}

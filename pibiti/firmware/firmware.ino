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

int16_t buffer[7];
String names[7] = {"\"AcX\":", ",\"AcY\":", ",\"AcZ\":", ",\"GyX\":", ",\"GyY\":", ",\"GyZ\":", ",\"Tmp\":"};

//-------------------------------------

int n_packets, sample_period, command;
long n_captures;

//-------------------------------------

ESP8266WiFiMulti wifiMulti;

WiFiClient espClient;
PubSubClient mqtt(IO_SERVER, 1883, callback, espClient);

Sensor mpu;
MyESP esp;

//-------------------------------------

void callback(char *topic, byte *payload, unsigned int length)
{
    StaticJsonDocument<256> mqtt_input;
    deserializeJson(mqtt_input, payload);

    command = mqtt_input["cmd"];
    n_packets = mqtt_input["npk"];
    n_captures = mqtt_input["ncp"];
    sample_period = mqtt_input["spe"];
}

//-------------------------------------

void setup()
{
    Serial.begin(500000);
    Serial.println("\n\nAnálise de vibração");

    Wire.begin(sda, scl);
    mpu.wakeup();

    esp.setWifi(wifiMulti);
}

//-------------------------------------

void loop()
{
    while (command == 1)
    {
        prevCheckTime = millis();

        for (int i = 0; i < n_packets; i++)
        {
            mqtt.loop();

            Serial.println("Entrou");

            mpu.read(0, captures);
            Serial.println("Captura ok");
            esp.sendData(justPrint);
            Serial.println("Envio ok");
        }
        command = -1;
    }

    prevCheckTime = millis();

    while (command == 2)
    { // Analise em tempo real

        currTime = millis();
        if ((currTime - prevCheckTime >= sample_period))
        {
            prevCheckTime = currTime;

            mpu.read();
            esp.print();
            esp.sendData();

            capt_counter++;
        }

        if (capt_counter >= n_captures)
        {
            command = -1;
            capt_counter = 0;
        }

        mqtt.loop();
    }

    if(command == -1)
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

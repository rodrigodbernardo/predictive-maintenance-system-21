#include <Wire.h>
#include <PubSubClient.h>
#include <ESP8266WiFiMulti.h>
#include <ArduinoJson.h>


#include "pibiti.h"
#include "secure.hpp"

//-------------------------------------

#define sda D6
#define scl D5

const uint8_t MPU_ADDR = 0x68;
const uint8_t WHO_AM_I = 0x75;
const uint8_t PWR_MGMT_1 = 0x6B;
const uint8_t GYRO_CONFIG = 0x1B;  // Registrador que configura a escala do giroscópio.
const uint8_t ACCEL_CONFIG = 0x1C; // Registrador que configura a escala do acelerômetro.
const uint8_t ACCEL_XOUT = 0x3B;   //
const uint8_t GYRO_SCALE = 8;      // Escala do giroscópio
const uint8_t ACCEL_SCALE = 8;     // Escala do acelerômetro

unsigned long prevCheckTime = 0;
unsigned long checkInterval = 100;

unsigned long currTime;


const int captures = 10;

int16_t buff[captures][7];   // Dado atual dos sensores acc e gyr no tipo raw (puro).

String names[7] = {"\"AcX\":", ",\"AcY\":", ",\"AcZ\":", ",\"GyX\":", ",\"GyY\":", ",\"GyZ\":", ",\"Tmp\":"};

bool justPrint = 1;
bool capture_done;

//-------------------------------------


String command = "";
int n_packets, sample_period;

//-------------------------------------

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
  Serial.println("Chegou Mensagem");

  StaticJsonDocument<256> doc;
  deserializeJson(doc, payload);

  command = (String)doc["command"];
  n_packets = doc["n_packets"];
  sample_period = doc["sample_period"];

  Serial.println(command);
  Serial.println(n_packets);
  Serial.println(sample_period);

}

//-------------------------------------
ESP8266WiFiMulti wifiMulti;

WiFiClient espClient;
PubSubClient mqtt(IO_SERVER, 1883, callback, espClient);

Sensor mpu;
MyESP esp;

//-------------------------------------

void setup() {
  Serial.begin(500000);
  Wire.begin(sda, scl);
  Serial.println("\n\nAnálise de vibração");

  mpu.wakeup();

  esp.setWifi(wifiMulti);


}

void loop() {

  if (command == "fast_capture") {
    prevCheckTime = millis();

    for (int i = 0; i < n_packets; i++) {

      mqtt.loop();


      Serial.println("Entrou");


      mpu.read(0, captures);
      Serial.println("Captura ok");
      esp.sendData(justPrint);
      Serial.println("Envio ok");



    }
    command = "stand-by";
  }

  prevCheckTime = millis();
  while (command == "live_capture") {

    currTime = millis();
    mqtt.loop();

    if ((currTime - prevCheckTime >= sample_period)) {
      prevCheckTime = currTime;

      mpu.read(justPrint, captures);
      esp.print();
    }
  }

  if (wifiMulti.run() != WL_CONNECTED) {
    esp.setWifi(wifiMulti);
  }

  if (!mqtt.connected()) {
    esp.setMqtt();
  }
  mqtt.loop();

}

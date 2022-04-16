#include <Wire.h>
#include <PubSubClient.h>
#include <ESP8266WiFiMulti.h>
#include <ArduinoJson.h>

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

unsigned long long prevCheckTime = 0;
unsigned long long checkInterval = 100;

unsigned long long currTime;

//-------------------------------------

int16_t buffer[7];
String names[7] = {"AcX:", ",AcY:", ",AcZ:", ",GyX:", ",GyY:", ",GyZ:", ",Tmp:"};

int16_t v_data[3000][7];

//-------------------------------------
int sample_period;
int n_packets, command;
long n_captures = 0;
long capt_counter = 0;
String status;

//-------------------------------------

void callback(char *topic, byte *payload, unsigned int length)
{
  StaticJsonDocument<256> mqtt_input;
  deserializeJson(mqtt_input, payload);

  command = mqtt_input["cmd"];
  n_packets = mqtt_input["npk"];
  n_captures = mqtt_input["ncp"];
  sample_period = mqtt_input["spe"];


  status  = "Comando recebido: ";
  status += command ;
  status += " -- ";
  status += n_packets;
  status += " pacotes -- ";
  status += n_captures;
  status += " capturas -- " ;
  status += sample_period;
  status += "ms ";

  Serial.println(status);
}

//-------------------------------------

ESP8266WiFiMulti wifiMulti;

WiFiClient espClient;
PubSubClient mqtt(IO_SERVER, 1883, callback, espClient);

void setup()
{
  Serial.begin(500000);
  delay(2000);
  Serial.println("\n\nAnálise de vibração");

  Wire.begin(sda, scl);

  sensor_wakeup();

  command = -1;
  n_packets = 10;
  n_captures = 1000;
  sample_period = 5000; //5 ms
}

void loop()
{
  if (wifiMulti.run() != WL_CONNECTED)
  {
    esp_setwifi();
  }

  mqtt.loop();

  if (!mqtt.connected())
  {
    esp_setmqtt();
  }

  switch (command)
  {
    case 1:
      {
        for (int packet = 0; packet < n_packets; packet++)
        {
          int interval = 0;
          prevCheckTime = micros();
          unsigned long long packet_begin = micros();

          while (capt_counter < n_captures)
          {
            currTime = micros();
            interval = currTime - prevCheckTime;

            if ( interval >= sample_period)
            {
              prevCheckTime = currTime;
              yield();

              sensor_read();
              sensor_store();

              capt_counter++;
            }
          }

          esp_send_data();

          mqtt.loop();

          unsigned long long packet_end = micros();
          Serial.print("Tempo do pacote: ");
          Serial.println((packet_end - packet_begin));

          capt_counter = 0;

          delay(2000);
        }
        command = -1;

        break;
      }

    case 2:
      {
          int interval = 0;
          prevCheckTime = micros();

          while (command == 2)
          {
            currTime = micros();
            interval = currTime - prevCheckTime;

            if ( interval >= sample_period)
            {
              prevCheckTime = currTime;

              Serial.printf("Intervalo atual: %i\n", interval);

              yield();

              sensor_read();
              sensor_send_data_continuous();
              mqtt.loop();
            }
          }

        break;
      }

    case -1:
      {
        Serial.println("Waiting for command");
        delay(1000);
        break;
      }

    default:
      break;
  }
}

//-------------------------------------
//-------------------------------------
//-------------------------------------

void sensor_write(int reg, int val)
{
  //
  // COMUNICACAO I2C COM O SENSOR - NAO MODIFICAR
  //
  Wire.beginTransmission(MPU_ADDR); // inicia comunicação com endereço do MPU6050
  Wire.write(reg);                  // envia o registro com o qual se deseja trabalhar
  Wire.write(val);                  // escreve o valor no registro
  Wire.endTransmission();           // termina a transmissão
}

void sensor_wakeup()
{
  //
  // INICIA A CONFIGURAÇÃO DO SENSOR
  //
  sensor_write(PWR_MGMT_1, 0);             // ACORDA O SENSOR
  sensor_write(GYRO_CONFIG, GYRO_SCALE);   // CONFIGURA A ESCALA DO GIROSCÓPIO - +-250 °/s -->
  sensor_write(ACCEL_CONFIG, ACCEL_SCALE); // CONFIGURA A ESCALA DO ACELERÔMETRO - +-4G
}

void sensor_read()
{
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(ACCEL_XOUT);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, (uint8_t)14);

  for (int axis = 0; axis < 7; axis++) // LÊ OS DADOS DE ACC
    buffer[axis] = Wire.read() << 8 | Wire.read();

}

void sensor_store()
{
  for (int axis = 0; axis < 7; axis++)
    v_data[capt_counter][axis] = buffer[axis];
}

void sensor_print()
{
  for (int j = 0; j < 7; j++)
  {
    Serial.print(names[j]);
    Serial.print(v_data[0][j]);
  }
  Serial.println();
}

//-------------------------------------

void esp_setwifi()
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
  }

  Serial.print("\nWi-Fi conectada. IP ");
  Serial.println(WiFi.localIP());
}

void esp_setmqtt()
{
  // Loop until we're reconnected
  while (!mqtt.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqtt.connect(clientId.c_str(), "cliente", "cliente"))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      // mqtt.publish(topico_saida, "hello world");
      // ... and resubscribe
      mqtt.subscribe(topico_entrada);
    }
    else
    {
      // Serial.print("failed, rc=");
      // Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void esp_send_data()
{
  mqtt.publish(topico_status, status.c_str());

  String out_msg = "";

  for (int i = 0; i < n_captures; i++) {
    out_msg = "";

    for (int j = 0; j < 7; j++)
    {
      out_msg += names[j];
      out_msg += v_data[i][j];
    }
    mqtt.publish(topico_saida, out_msg.c_str());
    Serial.println(out_msg);
  }
}

void sensor_send_data_continuous()
{
  String out_msg = "";

  for (int j = 0; j < 7; j++)
  {
    out_msg += names[j];
    out_msg += buffer[j];
  }
  mqtt.publish(topico_saida, out_msg.c_str());
  Serial.println(out_msg);
}

/*

  ATONAÇÕES

  Sobre o período amostral

  Durante testes, verifiquei que o ESP consegue capturar os dados do sensor com precisão até uma certa velocidade. Acima disso, ele começa a atrasar na entrega do dado.
  Esse atraso se deve, provavelmente, à limitação da comunicação I2C com o sensor.

  Dessa forma, os períodos amostrais mais recomendados são:

  1. 5ms   (200Hz) - 100% dos dados no tempo correto
  2. 4ms   (250Hz) - 99%  dos dados no tempo correto
  3. 2.5ms (400Hz) - 95%  dos dados no tempo correto

*/

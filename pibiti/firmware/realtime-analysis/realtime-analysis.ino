#include <Arduino.h>
#include <ESP8266WiFiMulti.h>
#include <Wire.h>
#include <EEPROM.h>
#include <ArduinoJson.h>

#include "ESP8266httpUpdate.h"

#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#include "secure.hpp"

#define Version "1.1.0.10"
#define MakeFirmwareInfo(k, v) "&_FirmwareInfo&k=" k "&v=" v "&FirmwareInfo_&"

/*************************** VARIAVEIS ************************************/

char *message;
//unsigned long prevUpdateTime = 0;
String otaUrl;
//uint8_t retries = 5;

int16_t zeros[6];  // Média dos dados da calibração. Utilizados como zero.
int16_t buff[6];   // Dado atual dos sensores acc e gyr no tipo raw (puro).
int16_t buff_temp; // Dado atual do sensor tmp no tipo raw (puro).
//float buff_[6];    // Dado atual dos sensores acc e gyr em unidade do SI (m/s^2 e grau/s).
//float buff_temp_;  // Dado atual do sensor tmp em unidade do SI (ºC).

int16_t gravityRAW;     // Gravidade em raw (varia de acordo com o range do acelerometro escolhido); pode ser substituído por uma simples formula, mas eu preferi assim
//float range_a, range_g; // Meio range dos sensores acc e gyr, respectivamente
//unsigned long previousMillis = 0;

int *eeIntPtr;
//byte num_a;

/*************************** REGISTRADORES DO SENSOR ***************************************/

const uint8_t MPU_ADDR = 0x68;
const uint8_t WHO_AM_I = 0x75;
const uint8_t PWR_MGMT_1 = 0x6B;
const uint8_t GYRO_CONFIG = 0x1B;  // Registrador que configura a escala do giroscópio.
const uint8_t ACCEL_CONFIG = 0x1C; // Registrador que configura a escala do acelerômetro.
const uint8_t ACCEL_XOUT = 0x3B;   //
const uint8_t GYRO_SCALE = 8;      // Escala do giroscópio
const uint8_t ACCEL_SCALE = 8;    // Escala do acelerômetro

/*************************** CONSTANTES              ***************************************/
unsigned long prevCaptureTime = 0;
unsigned long captureInterval = 1000;
//const long subsInterval = 10000;
//const long updateInterval = 20000;

//const float localGravity = 9.7803;

const float halfRange = 32768; // Metade do range de 16 bits
//const long interval = 100;

#define sda D6
#define scl D5

/*************************** Pinos E/S                ***************************************/



/************ OBJETOS ******************/

WiFiClient wifiClient;
WiFiClient wifiClientOTA;
ESP8266WiFiMulti wifiMulti;
Adafruit_MQTT_Client mqtt(&wifiClient, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** TÓPICOS MQTT ***************************************/

// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish fullCapture = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/AcX");

Adafruit_MQTT_Subscribe updateButton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/firmware-update");
Adafruit_MQTT_Subscribe onoffbutton  = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/onoff");

/*************************** CODIGO ************************************/

void setup() {
  Serial.begin(500000);
  Wire.begin(sda, scl);
  delay(1000);

  Serial.println("\n\nANALISE DE VIBRAÇÃO EM TEMPO REAL");
  Serial.print("VERSAO: ");
  Serial.println(Version);

  wifiSet(wifiMulti);

  sensorWakeUp();
  gravityRAW = halfRange / 4; // sensorSetRange(localGravity);
  sensorCalibrate(0);

  mqtt.subscribe(&onoffbutton);
  mqtt.subscribe(&updateButton);

  EEPROM.begin(32);
  eeIntPtr = (int *)EEPROM.getDataPtr();
  loadConfigs();

  //Serial.println(captureInterval);
  updateFirmware();
  updateConfigs();
}

void loop() {
  if (!mqtt.connected())
    mqttConnect();

  unsigned long currTime = millis();

  if (currTime - prevCaptureTime >= captureInterval) {
    prevCaptureTime = currTime;

    sensorRead(0);      // Faz uma leitura no sensor. Quando recebe 0 como argumento, realiza um no sensor com base na calibração
    sensorPrint(0);     // Printa a ultima captura na Serial. Caso o argumento seja 0, printa o valor bruto. Se for 1, printa o valor no SI
    //sensorSend(0);      // Envia a ultima captura por MQTT.
  }

  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  /*
     if(! mqtt.ping()) {
     mqtt.disconnect();
     }
  */

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
    if (subscription == &updateButton) {
      message = (char *)updateButton.lastread;

      if (strcmp(message, "update-firmware") == 0)
        updateFirmware();
      if (strcmp(message, "update-config") == 0)
        updateConfigs();
    }
  }
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

void updateFirmware() {
  otaUrl =  "http://otadrive.com/deviceapi/update?";
  otaUrl += MakeFirmwareInfo(ProductKey, Version);
  otaUrl += "&s=" + String(CHIPID);

  t_httpUpdate_return ret = ESPhttpUpdate.update(wifiClientOTA, otaUrl, Version);

  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.println("\nFalha no OTA. Autorize o dispositivo ou verifique as config.\n");
      break;
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("\nNenhuma atualização OTA disponível.\n");
      break;
  }
}

void saveConfigs() {
  Serial.printf("Periodo amostral: %d ms\n", captureInterval);
  eeIntPtr[1] = captureInterval;
  EEPROM.commit();
}

void loadConfigs() {
  // check configs initialized in eeprom or not
  if (eeIntPtr[0] != 0x4A) {
    // configs not initialized yet. write for first time
    eeIntPtr[0] = 0x4A; // memory sign
    saveConfigs();
  }
  else {
    // configs initialized and valid. read values
    captureInterval = eeIntPtr[1];
  }
}

// NECESSARIO
void updateConfigs() {
  WiFiClient client;
  HTTPClient http;

  otaUrl =  "http://otadrive.com/deviceapi/getconfig?";
  otaUrl += MakeFirmwareInfo(ProductKey, Version);
  otaUrl += "&s=" + String(CHIPID);

  client.setTimeout(1);
  http.setTimeout(1000);
  http.setTimeout(1000);

  Serial.println(otaUrl);

  if (http.begin(client, otaUrl)) {
    int httpCode = http.GET();
    //Serial.printf("http code: %d\n", httpCode);

    // httpCode will be negative on error
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      DynamicJsonDocument doc(512);
      deserializeJson(doc, payload);
      //Serial.printf("http content: %s\n", payload.c_str());
      Serial.println("\n\n!!! Configuração recebida !!!\n\n");

      if (doc.containsKey("captureInterval")) {
        captureInterval = doc["captureInterval"].as<int>();
        saveConfigs();
      }
    }
  }
}

// ESSENCIAL
void sensorWakeUp() {
  //
  // INICIA A CONFIGURAÇÃO DO SENSOR
  //
  sensorWrite(PWR_MGMT_1, 0);             //ACORDA O SENSOR
  sensorWrite(GYRO_CONFIG, GYRO_SCALE);   //CONFIGURA A ESCALA DO GIROSCÓPIO - +-250 °/s -->
  sensorWrite(ACCEL_CONFIG, ACCEL_SCALE); //CONFIGURA A ESCALA DO ACELERÔMETRO - +-4G
}
/*
  void sensorSetRange(const float gravity) {
  //
  // SETA O RANGE A SER UTILIZADO NOS SENSORES
  //

  switch (GYRO_SCALE) { // VERIFICA A ESCALA E SALVA O RANGE DO GIROSCÓPIO
    case 0:          //padrao
      range_g = 1 * 250; //+- 250°/s
      break;
    case 8:
      range_g = 2 * 250; //+- 500°/s
      break;
    case 16:
      range_g = 4 * 250; //+- 1000°/s
      break;
    case 24:
      range_g = 8 * 250; //+- 2000°/s
      break;
  }
  switch (ACCEL_SCALE) { // VERIFICA A ESCALA, SALVA O RANGE DO GIROSCOPIO E A CONSTANTE GRAVITACIONAL
    case 0:
      num_a = 2;
      break;
    case 8:
      num_a = 4;//padrao - +-4g
      break;
    case 16:
      num_a = 8; //+-8g
      break;
    case 24:
      num_a = 16;//+-16g
      break;
  }

  range_a = num_a * gravity;    // SETA O RANGE DA ACELERAÇÃO
  range_g
  gravityRAW = halfRange/num_a; // MUDA O VALOR, EM RAW, REFERENTE À CONSTANTE GRAVITACIONAL, DE ACORDO COM O RANGE ESCOLHIDO. USADO NA CALIBRAÇÃO

  //  //OTIMIZADO
  gravityRAW = halfRange/4;

  }
*/
// NAO ESSENCIAL, RECOMENDAVEL
void sensorCalibrate(const uint8_t baseAxis) {
  //
  // LÊ O SENSOR 10 VEZES PARA OBTER UMA MÉDIA. A MÉDIA É UTILIZADA COMO ZERO.
  // ISSO É NECESSÁRIO POR QUE DOIS SENSORES PODEM APRESENTAR DIFERENÇA DE LEITURA ENTRE SI,
  // O QUE PODE SER PREJUDICIAL QUANDO SE UTILIZA VARIOS SENSORES
  //
  for (int calibIteration = 0; calibIteration < 10; calibIteration++) {
    sensorRead(1);
    for (int axis = 0; axis < 6; axis++)
      zeros[axis] += buff[axis] / 10;
  }

  zeros[baseAxis] -= gravityRAW;

  // PARA QUE OS SENSORES DE ACELERAÇÃO SEJAM MAIS PROXIMOS DA REALIDADE, O SENSOR
  // INDICA COMO ZERO DO EIXO QUE ESTÁ APONTANDO PARA CIMA (ACX, NESSE CASO) UM VALOR QUE É
  // A MÉDIA OBTIDA ANTERIORMENTE MENOS A CONSTANTE GRAVITACIONAL.
  // ISSO FAZ COM QUE O ZERO DO SENSOR SEJA APROX. O ZERO DA VIDA REAL, E ELE MOSTRA A ACELERAÇÃO DA
  // GRAVIDADE CORRETAMENTE NO PLOT
}

void sensorRead(int offsetFlag)
{
  //
  // LÊ AS MEMÓRIAS DO SENSOR. UTILIZADA PARA SABER OS VALORES DE ACELERAÇÃO E GIRO ATUAIS
  // POR PADRÃO, É CONFIGURADA PARA RECEBER:
  //    - 6 BYTES DE ACELERAÇÃO (2 BYTES PARA CADA EIXO)
  //    - 2 BYTES DE TEMPERATURA
  //    - 6 BYTES DE GIRO       (2 BYTES PARA CADA EIXO)
  //
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(ACCEL_XOUT);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, (uint8_t)14);

  for (int i = 0; i < 3; i++) // LÊ OS DADOS DE ACC
    buff[i] = Wire.read() << 8 | Wire.read();

  buff_temp = Wire.read() << 8 | Wire.read(); //LÊ OS DADOS DE TEMP

  for (int i = 3; i < 6; i++) // LÊ OS DADOS DE GYRO
    buff[i] = Wire.read() << 8 | Wire.read();

  // NAO ESSENCIAL, RECOMENDAVEL
  if (offsetFlag == 0) { //CALIBRA O ZERO DO SENSOR, SE SOLICITADO PELO USUARIO. A TEMPERATURA NÃO É CALIBRADA
    for (int axis = 0; axis < 6; axis++)
      buff[axis] -= zeros[axis];
  }
}

//NAO ESSENCIAL
void sensorPrint(bool noRawFlag) {
  // IMPRESSAO DE DADOS NA SERIAL
  // SE NORAWFLAG = 0, ENVIA OS DADOS BRUTOS
  // SE NORAWFLAG = 1, ENVIA OS DADOS NO SI

  String names[7] = {"AcX:", ",AcY:", ",AcZ:", ",GyX:", ",GyY:", ",GyZ:", ",Tmp:"};

  for (int axis = 0; axis < 6; axis++) {
    Serial.print(names[axis]);
    /*
        switch (noRawFlag) {
          case 0:
            Serial.print(buff[axis]);
            break;
          case 1:
            sensorConvert();
            Serial.print(buff_[axis]);
            break;
        }
    */
    // CODIGO OTIMIZADO (NO LUGAR DO SWITCH)
    Serial.print(buff[axis]);

  }
  Serial.println();
}

// ESSENCIAL
void sensorSend(bool noRawFlag) {
  // ENVIO DE DADOS DO SENSOR POR MQTT
  // SE NORAWFLAG = 0, ENVIA OS DADOS BRUTOS
  // SE NORAWFLAG = 1, ENVIA OS DADOS NO SI

  char tempMsg[15];
  char message[75] = "";
  /*
    for (int axis = 0; axis < 6; axis++) {
      switch (noRawFlag) {
        case 0:
          sprintf(tempMsg, "%i", buff[axis]);
          break;
        case 1:
          sprintf(tempMsg, "%f", buff_[axis]);
          break;
      }
      strcat(message, tempMsg);
      strcat(message, ";");
    }
  */
  // CODIGO OTIMIZADO (NO LUGAR DO SWITCH)
  for (int axis = 0; axis < 6; axis++) {
    sprintf(tempMsg, "%i", buff[axis]);
    strcat(message, tempMsg);
    strcat(message, ";");
  }

  //sprintf(message,"%f",buff_[0]);

  fullCapture.publish(message);
}

// ESSENCIAL
void sensorWrite(int reg, int val) {
  // COMUNICACAO I2C COM O SENSOR - NAO MODIFICAR
  Wire.beginTransmission(MPU_ADDR); // inicia comunicação com endereço do MPU6050
  Wire.write(reg);                  // envia o registro com o qual se deseja trabalhar
  Wire.write(val);                  // escreve o valor no registro
  Wire.endTransmission();           // termina a transmissão
}

// NAO ESSENCIAL
/*
void sensorConvert() {
  // CONVERSAO DE DADOS BRUTOS PARA SI. ACC TEM UM RANGE DIFERENTE DE GYR
  for (int axis = 0; axis < 3; axis++)
    buff_[axis] = mapfloat(buff[axis], -halfRange, halfRange, -range_a, range_a);
  for (int axis = 3; axis < 6; axis++)
    buff_[axis] = mapfloat(buff[axis], -halfRange, halfRange, -range_g, range_g);

  //buff_[0] += gravity; --> nao descomentar

  buff_temp_ = (float)buff_temp / 340.00 + 36.53; // A TEMPERATURA TEM UMA FORMULA DE CONVERSAO DIFERENTE
}

// NAO ESSENCIAL
float mapfloat(long x, long in_min, long in_max, long out_min, long out_max) {
  // INTERPOLAÇÃO DE DADOS BRUTOS PARA SI
  return (float)(x - in_min) * (out_max - out_min) / (float)(in_max - in_min) + out_min;
}
*/

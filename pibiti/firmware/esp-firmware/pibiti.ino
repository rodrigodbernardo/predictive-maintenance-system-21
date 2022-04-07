#include <Wire.h>
#include "pibiti.h"

void Sensor::wakeup()
{
  //
  // INICIA A CONFIGURAÇÃO DO SENSOR
  //
  write(PWR_MGMT_1, 0);             // ACORDA O SENSOR
  write(GYRO_CONFIG, GYRO_SCALE);   // CONFIGURA A ESCALA DO GIROSCÓPIO - +-250 °/s -->
  write(ACCEL_CONFIG, ACCEL_SCALE); // CONFIGURA A ESCALA DO ACELERÔMETRO - +-4G
}

void Sensor::write(int reg, int val)
{
  //
  // COMUNICACAO I2C COM O SENSOR - NAO MODIFICAR
  //
  Wire.beginTransmission(MPU_ADDR); // inicia comunicação com endereço do MPU6050
  Wire.write(reg);                  // envia o registro com o qual se deseja trabalhar
  Wire.write(val);                  // escreve o valor no registro
  Wire.endTransmission();           // termina a transmissão
}

void Sensor::read(bool justPrint, int captures)
{
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(ACCEL_XOUT);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, (uint8_t)14);

  if (justPrint) {
    for (int j = 0; j < 7; j++) //LÊ OS DADOS DE ACC
      buff[0][j] = Wire.read() << 8 | Wire.read();
  } else {
    for ( int i = 0; i < captures; i++) {
      for (int j = 0; j < 7; i++) //LÊ OS DADOS DE ACC
        buff[i][j] = Wire.read() << 8 | Wire.read();
    }
    Serial.println("ok");
  }

}

void Sensor::print() {

  for ( int j = 0; j < 7; j++) {
    Serial.print(names[j]);
    Serial.print(buff[0][j]);
  }
  Serial.println();
}
/*
  void Sensor::calibrate(uint8_t baseAxis) {
  //
  // LÊ O SENSOR 10 VEZES PARA OBTER UMA MÉDIA. A MÉDIA É UTILIZADA COMO ZERO.
  // ISSO É NECESSÁRIO POR QUE DOIS SENSORES PODEM APRESENTAR DIFERENÇA DE LEITURA ENTRE SI,
  // O QUE PODE SER PREJUDICIAL QUANDO SE UTILIZA VARIOS SENSORES
  //
  //  baseAxis:
  //  0 = eixo X
  //  1 = eixo Y
  //  2 = eixo Z
  //
  for (int calibIteration = 0; calibIteration < 10; calibIteration++) {
    read(1,1,1,0);
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

  void Sensor::read(bool justCalibrating,bool offsetFlag, int capturesNumber, int samplePeriod)
  {
  //
  // LÊ AS MEMÓRIAS DO SENSOR. UTILIZADA PARA SABER OS VALORES DE ACELERAÇÃO E GIRO ATUAIS
  // POR PADRÃO, É CONFIGURADA PARA RECEBER:
  //    - 6 BYTES DE ACELERAÇÃO (2 BYTES PARA CADA EIXO)
  //    - 2 BYTES DE TEMPERATURA
  //    - 6 BYTES DE GIRO       (2 BYTES PARA CADA EIXO)
  //
  for (int capture = 0; capture < capturesNumber; iter++) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(ACCEL_XOUT);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_ADDR, (uint8_t)14);

    for (int i = 0; i < 3; i++) //LÊ OS DADOS DE ACC
      buff[i] = Wire.read() << 8 | Wire.read();

    buff_temp = Wire.read() << 8 | Wire.read(); //LÊ OS DADOS DE TEMP

    for (int i = 3; i < 6; i++) // LÊ OS DADOS DE GYRO
      buff[i] = Wire.read() << 8 | Wire.read();


    //  offsetFlag
    //  0 = calibra o sensor
    //

    if (offsetFlag == 0)
    { //CALIBRA O ZERO DO SENSOR, SE SOLICITADO PELO USUARIO. A TEMPERATURA NÃO É CALIBRADA
      for (int axis = 0; axis < 6; axis++)
        buff[axis] -= zeros[axis];
    }
  }
  }
*/
  //----------------------------------

  void MyESP::setWifi(ESP8266WiFiMulti wifiMulti)
  {
  WiFi.mode(WIFI_STA);

  wifiMulti.addAP(WLAN_SSID, WLAN_PASS);

  Serial.println("Conectando à rede Wi-Fi.");
  for (int retry = 15; (retry >= 0 && wifiMulti.run(1000) != WL_CONNECTED); retry--)
  {
    if (retry == 0)
      while (1)
        ;
    Serial.print(".");
  }

  Serial.print("\nWi-Fi conectada. IP ");
  Serial.println(WiFi.localIP());
  }

  void MyESP::setMqtt() {
  // Loop until we're reconnected
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqtt.connect(clientId.c_str(),"cliente","cliente")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      mqtt.publish(topico_saida, "hello world");
      // ... and resubscribe
      mqtt.subscribe(topico_entrada);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void MyESP::sendData(bool justPrint){
  
}

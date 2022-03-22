#include <Wire.h>
#include "sensor.h"

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
    read(1);
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

void Sensor::read(int offsetFlag)
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

//----------------------------------

void MyESP::setWifi(ESP8266WiFiMulti wifiMulti)
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
    delay(1000);
  }

  Serial.print("\nWi-Fi conectada. IP ");
  Serial.println(WiFi.localIP());
}

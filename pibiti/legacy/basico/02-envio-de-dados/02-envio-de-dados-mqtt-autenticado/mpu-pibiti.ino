#include <Wire.h>
#include "mpu-pibiti.h"

float mapfloat(long x, long in_min, long in_max, long out_min, long out_max)
{
  //
  // UTILIZADA SÓ PARA CONVERTER OS DADOS PARA UM VALOR AMIGAVEL PARA HUMANOS (SISTEMA INTERNACIONAL)
  //
  return (float)(x - in_min) * (out_max - out_min) / (float)(in_max - in_min) + out_min;
}

void sensor::setWifi(ESP8266WiFiMulti wifiMulti)
{

  delay(100);
  WiFi.mode(WIFI_STA);

  for (int i = 0; i < 1; i++)
    wifiMulti.addAP(ssid[i], password[i]);

  while (wifiMulti.run() != WL_CONNECTED)
  {
    Serial.println("Conectando à rede Wi-Fi.");
    delay(500);
  }

  Serial.print("\nWi-Fi conectada. IP ");
  Serial.println(WiFi.localIP());
  Serial.println(sizeof(*ssid));
}

void sensor::wakeUp()
{
  //
  // INICIA A CONFIGURAÇÃO DO SENSOR
  //
  write(PWR_MGMT_1, 0);             //ACORDA O SENSOR
  write(GYRO_CONFIG, GYRO_SCALE);   //CONFIGURA A ESCALA DO GIROSCÓPIO - +-250 °/s -->
  write(ACCEL_CONFIG, ACCEL_SCALE); //CONFIGURA A ESCALA DO ACELERÔMETRO - +-4G
}

void sensor::write(int reg, int val)
{
  //
  // COMUNICA-SE COM O SENSOR POR I2C. UTILIZADA POR OUTRAS FUNÇÕES
  //
  Wire.beginTransmission(MPU_ADDR); // inicia comunicação com endereço do MPU6050
  Wire.write(reg);                  // envia o registro com o qual se deseja trabalhar
  Wire.write(val);                  // escreve o valor no registro
  Wire.endTransmission();           // termina a transmissão
}

void sensor::read(int offsetFlag)
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

  if (offsetFlag == 0)
  { //CALIBRA O ZERO DO SENSOR, SE SOLICITADO PELO USUARIO. A TEMPERATURA NÃO É CALIBRADA
    for (int axis = 0; axis < 6; axis++)
      buff[axis] -= zeros[axis];
  }
}

void sensor::setRange(const float gravity)
{
  //
  // SETA O RANGE A SER UTILIZADO NOS SENSORES
  //
  switch (GYRO_SCALE)
  { // VERIFICA A ESCALA E SALVA O RANGE DO GIROSCÓPIO
    case 0:          //padrao
      range_g = 250; //+- 250°/s
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

  switch (ACCEL_SCALE)
  { // VERIFICA A ESCALA, SALVA O RANGE DO GIROSCOPIO E A CONSTANTE GRAVITACIONAL
    case 0:
      range_a = 2 * gravity;      // +-2g
      gravityRAW = halfRange / 2; // MUDA O VALOR, EM BITS, REFERENTE À CONSTANTE GRAVITACIONAL, DE ACORDO COM O RANGE ESCOLHID.
      break;
    case 8:
      range_a = 4 * gravity; //padrao - +-4g
      gravityRAW = halfRange / 4;
      break;
    case 16:
      range_a = 8 * gravity; //+-8g
      gravityRAW = halfRange / 8;
      break;
    case 24:
      range_a = 16 * gravity; //+-16g
      gravityRAW = halfRange / 16;
      break;
  }
}
void sensor::convert()
{
  //
  // CONVERTE OS VALORES DE NUMERO BRUTO PARA O SISTEMA INTERNACIONAL UTILIZANDO A FUNCAO QUE ESTÁ NO INICIO DO CODIGO
  //
  for (int axis = 0; axis < 3; axis++)
    buff_[axis] = mapfloat(buff[axis], -halfRange, halfRange, -range_a, range_a);
  for (int axis = 3; axis < 6; axis++)
    buff_[axis] = mapfloat(buff[axis], -halfRange, halfRange, -range_g, range_g);

  //buff_[0] += gravity; --> nao descomentar

  buff_temp_ = (float)buff_temp / 340.00 + 36.53; // A TEMPERATURA TEM UMA FORMULA DE CONVERSAO DIFERENTE
}
void sensor::calibrate(const uint8_t baseAxis)
{
  //
  // LÊ O SENSOR 10 VEZES PARA OBTER UMA MÉDIA. A MÉDIA É UTILIZADA COMO ZERO.
  // ISSO É NECESSÁRIO POR QUE DOIS SENSORES PODEM APRESENTAR DIFERENÇA DE LEITURA ENTRE SI,
  // O QUE PODE SER PREJUDICIAL QUANDO SE UTILIZA VARIOS SENSORES
  //
  for (int calibIteration = 0; calibIteration < 10; calibIteration++)
  {
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
void sensor::print(const bool noRawFlag)
{
  //
  // PRINTA OS VALORES EM FORMATO COMPATÍVEL COM O MONITOR SERIAL DO ARDUINO
  //
  String names[7] = {"AcX:", ",AcY:", ",AcZ:", ",GyX:", ",GyY:", ",GyZ:", ",Tmp:"};

  for (int axis = 0; axis < 6; axis++)
  {
    Serial.print(names[axis]);

    switch (noRawFlag)
    { //essa flag informa se vai ser printado o valor bruto ou convertido
      case 0: //printa o valor bruto
        Serial.print(buff[axis]);
        break;
      case 1: //printa o valor convertido
        sensor::convert();
        Serial.print(buff_[axis]);
        break;
    }
  }
  Serial.println();
}

void sensor::send(PubSubClient &client, const bool noRawFlag)
{
  char tempMsg[50];
  char message[100] = "";

  for (int axis = 0; axis < 6; axis++)
  {
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
  //sprintf(message,"%f",buff_[0]);

  client.publish(topico_teste_saida, message);
  client.loop();
}
/*
  void sensor::setBroker(PubSubClient &client) //setupMQTT
  {

  String deviceID = "ESP-";
  //deviceID += String(random(0xffff), HEX);
  deviceID += WiFi.macAddress();

  Serial.println("Conectando. ID: " + deviceID);
  if (client.connect(deviceID, mqttuser, mqttpass))
  {
    Serial.println("\nBroker ok!");
    client.subscribe(topico_teste_entrada);
  }
  else
  {
    Serial.println("Erro. Tentando novamente.");
    delay(5000);
  }
  }
*/

#include <Wire.h>
#include "mpu-pibiti.h"
#include "key.h"

float mapfloat(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (float)(x - in_min) * (out_max - out_min) / (float)(in_max - in_min) + out_min;
}

void sensor::setMqtt(PubSubClient& MQTT) {
  MQTT.setServer(broker_addr, broker_port);
  //MQTT.setCallback(dataInput);
}

void sensor::send(PubSubClient& MQTT) {
  /*
  for (int axis = 0; axis < 6; axis++) {
    //Serial.print(names[axis]);

    switch (rawFlag) { //essa flag informa se vai ser printado o valor bruto ou convertido
      case 0://printa o valor bruto
        Serial.print(buff[axis]);
        break;
      case 1://printa o valor convertido
        sensor::convert();
        Serial.print(buff_[axis]);
        break;
    }
  }*/

  char tempMsg[50];
  char message[100] = "";
  
  for (int axis = 0; axis < 6; axis++){
    sprintf(tempMsg,"%f",buff_[axis]);
    
    strcat(message,tempMsg);
    strcat(message,";");

  }
  //sprintf(message,"%f",buff_[0]);
  
  MQTT.publish(outTopic, message);
  MQTT.loop();
}

void sensor::setWifi(ESP8266WiFiMulti wifiMulti) {
  wifiMulti.addAP(ssid[0], password[0]);
  wifiMulti.addAP(ssid[1], password[1]);

  WiFi.mode(WIFI_STA);

  while (wifiMulti.run() != WL_CONNECTED) {

    Serial.println("Trying to connect to WiFi.");
    delay(500);
  }
  Serial.print("\nWiFi connected. IP ");
  Serial.println(WiFi.localIP());
}

void sensor::wakeup() {
  write(PWR_MGMT_1, 0);             //ACORDA O SENSOR
  write(GYRO_CONFIG, GYRO_SCALE);   //CONFIGURA A ESCALA DO GIROSCÓPIO - +-250 °/s -->
  write(ACCEL_CONFIG, ACCEL_SCALE); //CONFIGURA A ESCALA DO ACELERÔMETRO - +-4G
}

void sensor::write(int reg, int val) {
  Wire.beginTransmission(MPU_ADDR); // inicia comunicação com endereço do MPU6050
  Wire.write(reg);                  // envia o registro com o qual se deseja trabalhar
  Wire.write(val);                  // escreve o valor no registro
  Wire.endTransmission();           // termina a transmissão
}
void sensor::read(int calibFlag) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(ACCEL_XOUT);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, (uint8_t)14);

  for (int i = 0; i < 3; i++)
    buff[i]   = Wire.read() << 8 | Wire.read();

  buff_temp   = Wire.read() << 8 | Wire.read();

  for (int i = 3; i < 6; i++)
    buff[i]   = Wire.read() << 8 | Wire.read();

  if (calibFlag != 1) {
    for (int axis = 0; axis < 6; axis++)
      buff[axis] -= zeros[axis];
  }
}

void sensor::setRange() {

  switch (GYRO_SCALE) {
    case 0b00000000://padrao
      range_g = 250;
      break;
    case 0b00001000:
      range_g = 2 * 250;
      break;
    case 0b00010000:
      range_g = 4 * 250;
      break;
    case 0b00011000:
      range_g = 8 * 250;
      break;
  }

  switch (ACCEL_SCALE) {
    case 0b00000000:
      range_a = 2 * gravity;
      gravityConst = halfRange / 2;
      break;
    case 0b00001000://padrao
      range_a = 4 * gravity;
      gravityConst = 8192;//halfRange / 4;
      break;
    case 0b00010000:
      range_a = 8 * gravity;
      gravityConst = halfRange / 8;
      break;
    case 0b00011000:
      range_a = 16 * gravity;
      gravityConst = halfRange / 16;
      break;
  }
  Serial.println(gravityConst);
}

void sensor::convert() {

  for (int axis = 0; axis < 3; axis++)
    buff_[axis] = mapfloat(buff[axis], -halfRange, halfRange, -range_a, range_a);
  for (int axis = 3; axis < 6; axis ++)
    buff_[axis] = mapfloat(buff[axis], -halfRange, halfRange, -range_g, range_g);

  //buff_[0] += gravity;

  buff_temp_ = (float)buff_temp / 340.00 + 36.53;
}

void sensor::calibrate() {

  for (int calibIteration = 0; calibIteration < 10; calibIteration ++) {
    read(1);
    for (int axis = 0; axis < 6; axis++)
      zeros[axis] += buff[axis] / 10;
  }
  zeros[0] -= gravityConst;
}

void sensor::print() {
  String names[7] = {"AcX:", ",AcY:", ",AcZ:", ",GyX:", ",GyY:", ",GyZ:", ",Tmp:"};

  for (int axis = 0; axis < 6; axis++) {
    Serial.print(names[axis]);

    switch (rawFlag) { //essa flag informa se vai ser printado o valor bruto ou convertido
      case 0://printa o valor bruto
        Serial.print(buff[axis]);
        break;
      case 1://printa o valor convertido
        sensor::convert();
        Serial.print(buff_[axis]);
        break;
    }

  }
  /*
    Serial.print(names[6]);
    if (rawFlag)
    Serial.print(buff_temp_);
    else
    Serial.print(buff_temp);
  */

  Serial.println();
}

#include <Wire.h>
#include "sensor.h"

void Sensor::wakeup() {
  //
  // INICIA A CONFIGURAÇÃO DO SENSOR
  //
  write(PWR_MGMT_1, 0);             //ACORDA O SENSOR
  write(GYRO_CONFIG, GYRO_SCALE);   //CONFIGURA A ESCALA DO GIROSCÓPIO - +-250 °/s -->
  write(ACCEL_CONFIG, ACCEL_SCALE); //CONFIGURA A ESCALA DO ACELERÔMETRO - +-4G
}

void Sensor::write(int reg, int val) {
  //
  // COMUNICACAO I2C COM O SENSOR - NAO MODIFICAR
  //
  Wire.beginTransmission(MPU_ADDR); // inicia comunicação com endereço do MPU6050
  Wire.write(reg);                  // envia o registro com o qual se deseja trabalhar
  Wire.write(val);                  // escreve o valor no registro
  Wire.endTransmission();           // termina a transmissão
}

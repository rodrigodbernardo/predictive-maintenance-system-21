#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <Wire.h>

#define sda D6
#define scl D5

const uint8_t MPU_ADDR = 0x68;
const uint8_t WHO_AM_I = 0x75;
const uint8_t PWR_MGMT_1 = 0x6B;
const uint8_t GYRO_CONFIG = 0x1B;  //REGISTRADOR RESPONSAVEL POR CONFIGURAR A ESCALA DO GIROSCOPIO
const uint8_t ACCEL_CONFIG = 0x1C; //REGISTRADOR RESPONSAVEL POR CONFIGURAR A ESCALA DO ACELEROMETRO
const uint8_t ACCEL_XOUT = 0x3B;
const uint8_t GYRO_SCALE = 0b00000000; //+-250 graus/s -> ver documentação para saber a escala de conversao do sensor
const uint8_t ACCEL_SCALE = 0b00001000; //+- 4 g

int16_t zeroGyY;
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ; 
float AcX_, AcY_, AcZ_, Tmp_, GyX_, GyY_, GyZ_; 

ESP8266WiFiMulti wifiMulti;

void setup() {
  Serial.begin(500000);
  Wire.begin(sda, scl);
  sensorStart();
  wifiStart();
  calibrate();
}

void loop() {
  sensorRead(0);
  sensorConvert();
  Serial.print("AcX:");
  Serial.print(AcX_);
  Serial.print(",AcY:");
  Serial.print(AcY_);
  Serial.print(",AcZ:");
  Serial.print(AcZ_);
  Serial.print(",Tmp:");
  Serial.print(Tmp/340.00+36.53);
  Serial.print(",GyX:");
  Serial.print(GyX_);
  Serial.print(",GyY:");
  Serial.print(GyY_);
  Serial.print(",GyZ:");
  Serial.println(GyZ_);
  
  delay(10);

}

void sensorWrite(int reg, int val) {
  Wire.beginTransmission(MPU_ADDR); // inicia comunicação com endereço do MPU6050
  Wire.write(reg);                  // envia o registro com o qual se deseja trabalhar
  Wire.write(val);                  // escreve o valor no registro
  Wire.endTransmission();           // termina a transmissão
}

void sensorStart() {
  sensorWrite(PWR_MGMT_1, 0);             //ACORDA O SENSOR
  sensorWrite(GYRO_CONFIG, GYRO_SCALE);   //CONFIGURA A ESCALA DO GIROSCÓPIO - +-250 °/s
  sensorWrite(ACCEL_CONFIG, ACCEL_SCALE); //CONFIGURA A ESCALA DO ACELERÔMETRO - +-4G
}

void wifiStart() {
  wifiMulti.addAP("FLAVIO_02", "8861854611");

  WiFi.mode(WIFI_STA);

  while (wifiMulti.run() != WL_CONNECTED)
  {
    delay(100);
    Serial.print(".");
  }

  Serial.println("\n\nCONNECTED TO " + WiFi.SSID());
  Serial.print("IP ADDRESS: ");
  Serial.println(WiFi.localIP());
}

void sensorRead(int flag) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(ACCEL_XOUT);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, (uint8_t)14);

  AcX = Wire.read() << 8;                 // lê primeiro o byte mais significativo
  AcX |= Wire.read();                     // depois lê o bit menos significativo
  AcY = Wire.read() << 8;
  AcY |= Wire.read();
  AcZ = Wire.read() << 8;
  AcZ |= Wire.read();

  Tmp = Wire.read() << 8;
  Tmp |= Wire.read();

  GyX = Wire.read() << 8;
  GyX |= Wire.read();
  GyY = Wire.read() << 8;
  GyY |= Wire.read();
  GyZ = Wire.read() << 8;
  GyZ |= Wire.read();

  if(flag != 1){
    GyY = GyY - zeroGyY;
  }
}

void sensorConvert(){
  float gravity = 9.7803;
  float halfRange = 32768;
  float range_a, range_g;
  
  if(GYRO_SCALE == 0b00000000)
    range_g = 250;
  else if(GYRO_SCALE == 0b00001000)
    range_g = 2*250;
  else if(GYRO_SCALE == 0b00010000)
    range_g = 4*250;
  else if(GYRO_SCALE == 0b00011000)
    range_g = 8*250;

  if(ACCEL_SCALE == 0b00000000)
    range_a = 2*gravity;
  else if(ACCEL_SCALE == 0b00001000)
    range_a = 4*gravity;
  else if(ACCEL_SCALE == 0b00010000)
    range_a = 8*gravity;
  else if(ACCEL_SCALE == 0b00011000)
    range_a = 16*gravity;

  AcX_ = mapfloat(AcX,-halfRange,halfRange,-range_a,range_a);
  AcY_ = mapfloat(AcY,-halfRange,halfRange,-range_a,range_a);
  AcZ_ = mapfloat(AcZ,-halfRange,halfRange,-range_a,range_a);
  GyX_ = mapfloat(GyX,-halfRange,halfRange,-range_g,range_g);
  GyY_ = mapfloat(GyY,-halfRange,halfRange,-range_g,range_g);
  GyZ_ = mapfloat(GyZ,-halfRange,halfRange,-range_g,range_g);
}

float mapfloat(long x, long in_min, long in_max, long out_min, long out_max)
{
 return (float)(x - in_min) * (out_max - out_min) / (float)(in_max - in_min) + out_min;
}

void calibrate(){

  for(int calibIteration = 0; calibIteration < 100; calibIteration ++){
    sensorRead(1);
    if(calibIteration == 0)
      zeroGyY = GyY;
    
    zeroGyY = (zeroGyY + GyY)/2;
  }
}

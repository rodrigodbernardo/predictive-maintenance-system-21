#ifndef PIBITI_H
#define PIBITI_H

class Sensor
{
public:
  void wakeup();
  void write(int reg, int val);
  void calibrate(uint8_t baseAxis);
  void read(bool justCalibrating,bool offsetFlag, int capturesNumber, int samplePeriod);
};

class MyESP
{
public:
  void setWifi(ESP8266WiFiMulti wifiMulti);
};

#endif

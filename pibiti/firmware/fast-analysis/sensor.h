#ifndef PIBITI_H
#define PIBITI_H

class Sensor
{
public:
  void wakeup();
  void write(int reg, int val);
  void calibrate(uint8_t baseAxis);
};

class MyESP
{
public:
  void setWifi(ESP8266WiFiMulti wifiMulti);
};

#endif

#ifndef PIBITI_H
#define PIBITI_H

class Sensor
{
public:
  void wakeup();
  void write(int reg, int val);
  void read(bool justPrint, int captures);
  void print();
};

class MyESP
{
public:
  void setWifi(ESP8266WiFiMulti wifiMulti);
  void setMqtt();
  void sendData(bool justPrint);
};

#endif

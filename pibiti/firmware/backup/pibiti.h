#ifndef PIBITI_H
#define PIBITI_H

class Sensor
{
  public:
    void wakeup();
    void write(int reg, int val);
    void read();

};

class MyESP
{
  public:
    void setWifi(ESP8266WiFiMulti wifiMulti);
    void setMqtt();
    void sendData();
    void sendStoredData();
    //void print();
};

#endif

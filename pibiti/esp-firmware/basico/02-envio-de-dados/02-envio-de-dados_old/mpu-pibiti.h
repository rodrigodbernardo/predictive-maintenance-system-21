#ifndef PIBITI_H
#define PIBITI_H

class sensor {
  public:
    void setMqtt(PubSubClient& MQTT);
    void setWifi(ESP8266WiFiMulti wifiMulti);
    void send(PubSubClient& MQTT);
    void wakeup();
    void write(int reg, int val);
    void read(int flag);
    void setRange();
    void convert();
    void calibrate();
    void print();
};

#endif

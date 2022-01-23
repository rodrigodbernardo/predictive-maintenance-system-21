#ifndef PIBITI_H
#define PIBITI_H

class sensor {
  public:
    //void setMqtt(PubSubClient& client);
    void setWifi(ESP8266WiFiMulti wifiMulti);
    void send(PubSubClient& client, const bool noRawFlag);
    void wakeUp();
    void write(int reg, int val);
    void read(int offsetFlag);
    void setRange(const float gravity);
    void convert();
    void calibrate(const uint8_t baseAxis);
    void print(const bool noRawFlag);
    void setBroker(PubSubClient& client);
};

#endif

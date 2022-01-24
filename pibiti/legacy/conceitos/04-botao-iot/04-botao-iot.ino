#include <Arduino.h>
#include <ESP8266WiFiMulti.h>

#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#include "secure.hpp"

/************ Global State (you don't need to change this!) ******************/

WiFiClient wifiClient;
ESP8266WiFiMulti wifiMulti;

Adafruit_MQTT_Client mqtt(&wifiClient, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>

Adafruit_MQTT_Publish button = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/onoff");

/*************************** VARIABLES ************************************/


String firmware_version = "1.0.0.0";
bool led_state = LOW;
char *message;

/*************************** Sketch Code ************************************/

void setup()
{
    pinMode(D3, INPUT);

    Serial.begin(500000);
    delay(1000);

    Serial.println("BOTAO IOT");
    Serial.println("VERSAO " + firmware_version);

    setWifi(wifiMulti);
}

void loop()
{
    MQTT_connect();

    if (digitalRead(D3) == 1)
    {

        if (button.publish("led-switch"))
        {
            Serial.println("message sent");
        }
        else
        {
            Serial.println("error to send message");
        }
        delay(500);
    }

    // ping the server to keep the mqtt connection alive
    // NOT required if you are publishing once every KEEPALIVE seconds
    /*
    if(! mqtt.ping()) {
    mqtt.disconnect();
    }
  */
/*
    Adafruit_MQTT_Subscribe *subscription;
    while ((subscription = mqtt.readSubscription(10)))
    {

        if (subscription == &onoffbutton)
        {
            message = (char *)onoffbutton.lastread;
            Serial.println(message);
        }
    }
    */
}

void MQTT_connect()
{

    if (mqtt.connected())
        return;

    Serial.print("Connecting to MQTT... ");

    uint8_t retries = 5;
    while ((mqtt.connect()) != 0)
    { // connect will return 0 for connected
        Serial.println("Retrying MQTT connection in 5 seconds...");
        mqtt.disconnect();
        delay(5000); // wait 5 seconds
        retries--;

        if (retries == 0)
            while (1)
                ;
    }
    Serial.println("MQTT Connected!");
}

void setWifi(ESP8266WiFiMulti wifiMulti)
{

    delay(100);
    WiFi.mode(WIFI_STA);
    wifiMulti.addAP(WLAN_SSID, WLAN_PASS);

    do
    {
        Serial.println("Conectando à rede Wi-Fi.");
        delay(500);
    } while (wifiMulti.run() != WL_CONNECTED);

    Serial.print("\nWi-Fi conectada. IP ");
    Serial.println(WiFi.localIP());
}

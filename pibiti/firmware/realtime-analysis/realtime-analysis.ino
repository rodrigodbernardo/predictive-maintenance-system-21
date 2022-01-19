#include <ESP8266WiFiMulti.h>

#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/************************* WIFI SETUP   *********************************/

#define WLAN_SSID       "FLAVIO 02"
#define WLAN_PASS       "8861854611"

/************************* BROKER SETUP *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "rodrigodbernardo"
#define AIO_KEY         "aio_ozft64x6inK8IKbpWFcY5RNUzjp2"

/************ Global State (you don't need to change this!) ******************/

WiFiClient wifiClient;
ESP8266WiFiMulti wifiMulti;

Adafruit_MQTT_Client mqtt(&wifiClient, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>

Adafruit_MQTT_Publish AcX = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/AcX");
Adafruit_MQTT_Publish AcY = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/AcY");
Adafruit_MQTT_Publish AcZ = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/AcZ");

Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/onoff");

/*************************** VARIABLES ************************************/

#define RLED D3
#define GLED D2
#define YLED D1

const long publInterval = 10000;
const long subsInterval = 10000;

unsigned long prevPublTime = 0;
unsigned long prevSubsTime = 0;

String firmware_version = "1.0.0.0";
bool led_state = LOW;
char* message;

/*************************** Sketch Code ************************************/

void setup() {
  Serial.begin(500000);
  delay(1000);

  Serial.println("ANALISE DE VIBRAÇÃO EM TEMPO REAL");
  Serial.println("VERSAO " + firmware_version);

  setWifi(wifiMulti);

  mqtt.subscribe(&onoffbutton);

  pinMode(RLED, OUTPUT);
  pinMode(GLED, OUTPUT);
  pinMode(YLED, OUTPUT);
}

void loop() {
  MQTT_connect();

  unsigned long currTime = millis();

  if (currTime - prevPublTime >= publInterval) {

    prevPublTime = currTime;

    if (AcX.publish(led_state)) {
      Serial.println("OK");
    }
  }

  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  /*
    if(! mqtt.ping()) {
    mqtt.disconnect();
    }
  */


  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(10))) {

    if (subscription == &onoffbutton) {
      message = (char *)onoffbutton.lastread;

      if (strcmp(message, "RON") == 0) {
        digitalWrite(RLED, HIGH);
      } else if(strcmp(message, "ROFF") == 0){
        digitalWrite(RLED, LOW);
      } else if(strcmp(message, "GON") == 0){
        digitalWrite(GLED, HIGH);
      } else if(strcmp(message, "GOFF") == 0){
        digitalWrite(GLED, LOW);
      } else if(strcmp(message, "YON") == 0){
        digitalWrite(YLED, HIGH);
      } else if(strcmp(message, "YOFF") == 0){
        digitalWrite(YLED, LOW);
      }
    }
  }

}

void MQTT_connect() {

  if (mqtt.connected())
    return;

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 5;
  while ((mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
    retries--;

    if (retries == 0)
      while (1);
  }
  Serial.println("MQTT Connected!");
}

void setWifi(ESP8266WiFiMulti wifiMulti)
{

  delay(100);
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WLAN_SSID, WLAN_PASS);

  do {
    Serial.println("Conectando à rede Wi-Fi.");
    delay(500);
  } while (wifiMulti.run() != WL_CONNECTED);

  Serial.print("\nWi-Fi conectada. IP ");
  Serial.println(WiFi.localIP());
}

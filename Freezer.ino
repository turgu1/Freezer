
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "local/local_params.h"

// ---- Development Mode ----

#define debugging 0

#if debugging
  #define init_debug() { Serial.begin(9600);  }
  #define debug(msg)   { Serial.print(msg);   }
  #define debugln(msg) { Serial.println(msg); }
#else
  #define init_debug()
  #define debug(msg)
  #define debugln(msg)
#endif

// ---- Sensor and LED ----

#define LED   16
#define SWITCH 5

// Create an ESP8266 WiFiClient class to connect to the MQTT server.

// ---- MQTT Server Parameters ----

#define MQTT_ID      "freezerDoorSensor" " " __DATE__ " " __TIME__

#define FREEZER_FEED "/feeds/freezer_door"

// ---- Global State (you don't need to change this!) ----

WiFiClient   wifi_client;
PubSubClient mqtt(wifi_client);

void setup()
{
  delay(200);
  
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
  
  pinMode(SWITCH, INPUT_PULLUP);
  digitalWrite(SWITCH, HIGH);

  init_debug();
  delay(10);
}

void loop()
{ 
  // Ensure that MQTT is still connected

  MQTT_connect();
  
  // Publish data

  bool door_open = digitalRead(SWITCH) == HIGH;

  debug("Freezer Door: ");
  debugln(door_open ? "OPEN" : "CLOSE"); 
  
  MQTT_publish(FREEZER_FEED, door_open ? "OPEN" : "CLOSE");

  digitalWrite(LED, door_open ? LOW : HIGH);

  for (int i = 0; i < 5; i++) {
    delay(60000UL);
  }
}

// ---- blink() ----

void blink()
{
  digitalWrite(LED, LOW);
  delay(500);
  digitalWrite(LED, HIGH);
}

// ---- WIFI_connect() ----

void WIFI_connect()
{
  if (WiFi.status() == WL_CONNECTED) return;

  // Connect to WiFi access point.

  debug(F("Connecting to "));
  debugln(WLAN_SSID);

  debug("MAC: ");
  debugln(WiFi.macAddress());
  
  do {
    int count = 20;

    WiFi.mode(WIFI_STA);
    WiFi.begin(WLAN_SSID, WLAN_PASSWD);
    delay(1);

    while ((count--) && (WiFi.status() != WL_CONNECTED)) {
      delay(1000);
      debug(F("."));
    }
    debugln();

    if (WiFi.status() != WL_CONNECTED) {
      //WiFi.mode(WIFI_OFF);
      delay(500);
    }
  } while (WiFi.status() != WL_CONNECTED);
  
  debugln(F("WiFi connected"));
  
  debug(F("IP address: "  ));
  debugln(WiFi.localIP());  
}

// ---- MQTT_publish(feed, val) ----

void MQTT_publish(const char * feed, const char * msg)
{
  debugln(F("Publishing the following message"));
  debugln(msg);
  if (!mqtt.publish(feed, msg)) {
    debugln(F("Unable to publish message"));
  }
}

// ---- MQTT_connect() ----

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.

void MQTT_connect() 
{
  // Return if already connected.

  if (mqtt.loop()) return;
  
  WIFI_connect();
  
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  
  debug(F("Trying to connect to MQTT Server with ID: "));
  debugln(MQTT_ID);
  
  while (!mqtt.connect(MQTT_ID)) { // connect will return 1 for connected

    switch (mqtt.state()) {
      case -4: debugln(F("Connection Timeout")); break;
      case -3: debugln(F("Connection Losted" )); break;
      case -2: debugln(F("Connect Failed"    )); break;
      case -1: debugln(F("Disconnected"      )); break;
      case  1: debugln(F("Wrong protocol"    )); break;
      case  2: debugln(F("ID rejected"       )); break;
      case  3: debugln(F("Server unavail"    )); break;
      case  4: debugln(F("Bad user/pass"     )); break;
      case  5: debugln(F("Not authed"        )); break;
      default: debugln(F("Unknown error"     )); break;
    }
    
    delay(5000);  // wait 5 seconds

    WIFI_connect();
  }

  debugln(F("Connected"));
}


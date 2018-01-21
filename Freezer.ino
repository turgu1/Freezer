
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "local/local_params.h"

#define LED   16
#define SWITCH 5

// Create an ESP8266 WiFiClient class to connect to the MQTT server.

WiFiClient client;

// ---- MQTT Server Parameters ----

#define MQTT_ID          "freezerDoorSensor" " " __DATE__ " " __TIME__

#define FREEZER_FEED "/feeds/freezer_door"

// ---- Global State (you don't need to change this!) ----

PubSubClient mqtt(client);

//#define debug(msg)   { Serial.print(msg);   }
//#define debugln(msg) { Serial.println(msg); }

#define debug(msg)
#define debugln(msg)

void WIFI_connect();
void MQTT_connect();
void MQTT_publish(const char * feed, const char * msg);
void blink_led();

void setup()
{
  delay(2000);
  
  // put your setup code here, to run once:
  
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
  
  pinMode(SWITCH, INPUT_PULLUP);
  digitalWrite(SWITCH, HIGH);

  Serial.begin(9600);
  
  WIFI_connect();
  
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  
  MQTT_connect();
}

void loop()
{ 
  // Ensure that MQTT is still connected

  if (WiFi.status() != WL_CONNECTED) WIFI_connect();
  if (!mqtt.loop()) MQTT_connect();
  
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
  // Connect to WiFi access point.
  delay(10);

  debug("MAC: ");
  debugln(WiFi.macAddress());
  
  debug(F("Connecting to "));
  debugln(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    debug(F("."));
  }
  debugln();

  debugln(F("WiFi connected"));
  debugln(F("IP address: "));
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
  
  int8_t ret;

  // Stop if already connected.

  if (mqtt.connected()) return;

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
  }

  debugln(F("Connected"));
}


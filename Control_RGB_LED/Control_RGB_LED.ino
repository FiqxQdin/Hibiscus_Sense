// To control RGB LED from Favoriot IoT Platform
// Protocol : MQTT

#include <WiFi.h>
#include <MQTT.h>
#include <NetworkClientSecure.h>
#include <Adafruit_NeoPixel.h>
#include "FavoriotCA.h"

const char ssid[] = "YOUR_WIFI_SSID";
const char password[] = "YOUR_WIFI_PASSWORD";
const char deviceDeveloperId[] = "YOUR_DEVICE_DEVELOPER_ID";
const char deviceAccessToken[] = "YOUR_DEVICE_ACCESS_TOKEN";
const char publishTopic[] = "/v2/streams";
const char rgbTopic[] = "/v2/rpc";

int redValue = 0;
int greenValue = 0;
int blueValue = 0;

Adafruit_NeoPixel rgb(1, 16);

unsigned long lastMillis = 0;

NetworkClientSecure client;
MQTTClient mqtt(4096);

void connectToWiFi() {
  Serial.print("Connecting to Wi-Fi '" + String(ssid) + "' ...");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println(" connected!");
}

void messageReceived(String &topic, String &payload) {

  Serial.println("Incoming Status: " + payload);

  if (payload.indexOf("\"red\":") >= 0) {
    int startIndex = payload.indexOf(":\"") + 2; 
    int endIndex = payload.indexOf("\"", startIndex); 
    String redValueStr = payload.substring(startIndex, endIndex); 
    redValue = redValueStr.toInt(); 
    Serial.print("Red Slider Value: ");
    Serial.print(redValue);
  }
  else if (payload.indexOf("\"green\":") >= 0) {
    int startIndex = payload.indexOf(":\"") + 2; 
    int endIndex = payload.indexOf("\"", startIndex); 
    String green_valueStr = payload.substring(startIndex, endIndex); 
    greenValue = green_valueStr.toInt(); 
    Serial.print("Green Slider Value: ");
    Serial.print(greenValue);
  }
  else if (payload.indexOf("\"blue\":") >= 0) {
    int startIndex = payload.indexOf(":\"") + 2; 
    int endIndex = payload.indexOf("\"", startIndex); 
    String blue_valueStr = payload.substring(startIndex, endIndex); 
    blueValue = blue_valueStr.toInt(); 
    Serial.print("Blue Slider Value: ");
    Serial.print(blueValue);
  }

  rgb.setPixelColor(0, redValue, greenValue, blueValue);
  rgb.show();

  Serial.println();
}

void connectToFavoriotMQTT() {
  Serial.print("Connecting to Favoriot MQTT ...");

  client.setCACert(rootCACertificate);

  mqtt.begin("mqtt.favoriot.com", 8883, client);
  mqtt.onMessage(messageReceived);

  String uniqueString = String(ssid) + "-" + String(random(1, 98)) + String(random(99, 999));
  char uniqueClientID[uniqueString.length() + 1];

  uniqueString.toCharArray(uniqueClientID, uniqueString.length() + 1);

  while (!mqtt.connect(uniqueClientID, deviceAccessToken, deviceAccessToken)) {
    Serial.print(".");
    delay(500);
  }

  Serial.println(" connected!");

  Serial.println("Subscribe to: " + String(deviceAccessToken) + String(rgbTopic));

  mqtt.subscribe(String(deviceAccessToken) + String(rgbTopic));
  Serial.println();
}

void setup() {
  Serial.begin(115200);

  rgb.begin();
  rgb.show();

  pinMode(2, OUTPUT); // to turn off blue led
  digitalWrite(2, HIGH); // to turn off blue led

  Serial.println();

  // STEP 1: Initialized Wi-Fi conectivity
  connectToWiFi();
  // STEP 2: Initialized MQTT connection to Favoriot MQTT broker
  connectToFavoriotMQTT();
}

void loop() {
  // Check Wi-Fi connection
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }

  // Check MQTT connection
  if (!mqtt.connected()) {
    connectToFavoriotMQTT();
  }

  mqtt.loop();
  delay(10);  // <- fixes some issues with WiFi stability

}

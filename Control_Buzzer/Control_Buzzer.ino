// To control Buzzer from Favoriot IoT Platform
// Protocol : MQTT

#include <WiFi.h>
#include <MQTT.h>
#include <NetworkClientSecure.h>
#include "tones.h"
#include "FavoriotCA.h"

const char ssid[] = "YOUR_WIFI_SSID";
const char password[] = "YOUR_WIFI_PASSWORD";
const char deviceDeveloperId[] = "YOUR_DEVICE_DEVELOPER_ID";
const char deviceAccessToken[] = "YOUR_DEVICE_ACCESS_TOKEN";
const char publishTopic[] = "/v2/streams";
const char buzzerTopic[] = "/v2/rpc";

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

  if (payload.indexOf("\"buzzer\":\"on\"") >= 0) {
    tone(13, NOTE_D4);
    Serial.println("Buzzer turned ON");
  } 
  else if (payload.indexOf("\"buzzer\":\"off\"") >= 0) {
    noTone(13);
    Serial.println("Buzzer turned OFF");
  }

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

  Serial.println("Subscribe to: " + String(deviceAccessToken) + String(buzzerTopic));

  mqtt.subscribe(String(deviceAccessToken) + String(buzzerTopic));
  Serial.println();
}

void setup() {
  Serial.begin(115200);

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

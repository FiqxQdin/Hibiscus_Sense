#include <WiFi.h>
#include <MQTT.h>
#include <NetworkClientSecure.h>
#include <Adafruit_BME280.h>
#include "FavoriotCA.h"

const char ssid[] = "YOUR_WIFI_SSID";
const char password[] = "YOUR_WIFI_PASSWORD";
const char deviceDeveloperId[] = "YOUR_DEVICE_DEVELOPER_ID";
const char deviceAccessToken[] = "YOUR_DEVICE_ACCESS_TOKEN";
const char publishTopic[] = "/v2/streams";
const char statusTopic[] = "/v2/streams/status";

Adafruit_BME280 bme;

// https://meteologix.com/my/observations/pressure-qnh.html
float hPaSeaLevel = 1015.00;

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

  Serial.println("Subscribe to: " + String(deviceAccessToken) + String(statusTopic));

  mqtt.subscribe(String(deviceAccessToken) + String(statusTopic));
  Serial.println();
}

void setup() {
  Serial.begin(115200);

  pinMode(2, OUTPUT); // to turn off blue led
  digitalWrite(2, HIGH); // to turn off blue led

  if (!bme.begin()) {
    Serial.println("Failed to find Hibiscus Sense BME280 chip");
  }

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

  // STEP 3: Data Acquisition - Read data from the sensors
    float humidity = bme.readHumidity();
    float temperature = bme.readTemperature();
    float barometer = bme.readPressure() / 100.00;
    float altitude = bme.readAltitude(hPaSeaLevel);

    Serial.print("Relative Humidity: ");
    Serial.print(humidity);
    Serial.println(" %RH");

    Serial.print("Approx. Altitude: ");
    Serial.print(altitude);
    Serial.println(" m");

    Serial.print("Barometric Pressure: ");
    Serial.print(barometer);
    Serial.println(" Pa");

    Serial.print("Ambient Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");

  // STEP 4: Data Ingestion - Send data to Favoriot's data stream using secure MQTT connection
  // Interval 15 seconds
  if (millis() - lastMillis > 15000) {
    lastMillis = millis();

    String favoriotJson = "{\"device_developer_id\":\"" + String(deviceDeveloperId) + "\",\"data\":{";
    favoriotJson += "\"humidity\":\"" + String(humidity) + "\",";
    favoriotJson += "\"altitude\":\"" + String(altitude) + "\",";
    favoriotJson += "\"barometer\":\"" + String(barometer) + "\",";
    favoriotJson += "\"temperature\":\"" + String(temperature) + "\"";
    favoriotJson += "}}";

    Serial.println("\nSending data to Favoriot's Data Stream ...");

    Serial.println("Data to Publish: " + favoriotJson);
    Serial.println("Publish to: " + String(deviceAccessToken) + String(publishTopic));

    mqtt.publish(String(deviceAccessToken) + String(publishTopic), favoriotJson);
  }

  Serial.println("=============================================");
  delay(3000);
}

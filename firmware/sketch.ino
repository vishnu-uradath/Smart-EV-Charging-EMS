#include <WiFi.h>
#include <PubSubClient.h>
#include "DHTesp.h"

// ====================================================
// PIN DEFINITIONS
// ====================================================

const int DHT_PIN = 15;
const int SOIL_PIN = 34;
const int LIGHT_PIN = 35;
const int RELAY_PIN = 5;

// ====================================================
// IRRIGATION SETTINGS
// ====================================================

const int PUMP_ON_LEVEL = 35;
const int PUMP_OFF_LEVEL = 55;

// ====================================================
// WIFI SETTINGS
// ====================================================

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

// ====================================================
// MQTT SETTINGS
// ====================================================

const char* MQTT_SERVER = "broker.hivemq.com";
const int MQTT_PORT = 1883;

// Unique topic prefix for our project
const char* TOPIC_TEMP =
    "wokwi-smart-agri-a7f3/temperature";

const char* TOPIC_HUMIDITY =
    "wokwi-smart-agri-a7f3/humidity";

const char* TOPIC_SOIL =
    "wokwi-smart-agri-a7f3/soil";

const char* TOPIC_LIGHT =
    "wokwi-smart-agri-a7f3/light";

const char* TOPIC_PUMP =
    "wokwi-smart-agri-a7f3/pump";

// ====================================================
// OBJECTS
// ====================================================

DHTesp dhtSensor;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

bool pumpState = false;

// Controls how often data is published
unsigned long lastPublishTime = 0;

const unsigned long PUBLISH_INTERVAL = 3000;

// ====================================================
// CONNECT TO WIFI
// ====================================================

void connectWiFi() {

  Serial.print("Connecting to WiFi");

  WiFi.begin(
    WIFI_SSID,
    WIFI_PASSWORD,
    6
  );

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected!");

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// ====================================================
// CONNECT TO MQTT
// ====================================================

void connectMQTT() {

  while (!mqttClient.connected()) {

    Serial.print("Connecting to MQTT... ");

    // Create a client ID
    String clientId =
        "ESP32-SmartAgri-";

    clientId +=
        String(random(0xffff), HEX);

    if (mqttClient.connect(clientId.c_str())) {

      Serial.println("CONNECTED");

    } else {

      Serial.print("FAILED, state = ");
      Serial.println(mqttClient.state());

      Serial.println(
        "Trying again in 2 seconds..."
      );

      delay(2000);
    }
  }
}

// ====================================================
// SETUP
// ====================================================

void setup() {

  Serial.begin(115200);

  randomSeed(micros());

  // DHT22
  dhtSensor.setup(
    DHT_PIN,
    DHTesp::DHT22
  );

  // Pins
  pinMode(SOIL_PIN, INPUT);
  pinMode(LIGHT_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);

  // IMPORTANT:
  // In your actual Wokwi circuit:
  // LOW = pump OFF
  // HIGH = pump ON
  digitalWrite(RELAY_PIN, LOW);

  Serial.println();
  Serial.println(
    "================================"
  );

  Serial.println(
    " SMART AGRICULTURE MQTT SYSTEM"
  );

  Serial.println(
    "================================"
  );

  // Connect WiFi
  connectWiFi();

  // Configure MQTT
  mqttClient.setServer(
    MQTT_SERVER,
    MQTT_PORT
  );
}

// ====================================================
// MAIN LOOP
// ====================================================

void loop() {

  // Reconnect WiFi if disconnected
  if (WiFi.status() != WL_CONNECTED) {

    connectWiFi();
  }

  // Reconnect MQTT if disconnected
  if (!mqttClient.connected()) {

    connectMQTT();
  }

  mqttClient.loop();

  // Publish every 3 seconds
  if (
    millis() - lastPublishTime
    >= PUBLISH_INTERVAL
  ) {

    lastPublishTime = millis();

    // --------------------------------
    // READ SENSORS
    // --------------------------------

    TempAndHumidity data =
        dhtSensor.getTempAndHumidity();

    int soilRaw =
        analogRead(SOIL_PIN);

    int lightRaw =
        analogRead(LIGHT_PIN);

    // Soil percentage
    int soilPercent =
        map(
          soilRaw,
          0,
          4095,
          0,
          100
        );

    soilPercent =
        constrain(
          soilPercent,
          0,
          100
        );

    // --------------------------------
    // AUTOMATIC IRRIGATION
    // --------------------------------

    if (
      soilPercent
      <= PUMP_ON_LEVEL
    ) {

      pumpState = true;
    }

    else if (
      soilPercent
      >= PUMP_OFF_LEVEL
    ) {

      pumpState = false;
    }

    // YOUR relay behaviour
    digitalWrite(
      RELAY_PIN,
      pumpState ? HIGH : LOW
    );

    // --------------------------------
    // SERIAL MONITOR
    // --------------------------------

    Serial.println();
    Serial.println(
      "--------------------------------"
    );

    Serial.print(
      "Temperature   : "
    );

    Serial.print(
      data.temperature,
      1
    );

    Serial.println(" C");


    Serial.print(
      "Humidity      : "
    );

    Serial.print(
      data.humidity,
      1
    );

    Serial.println(" %");


    Serial.print(
      "Soil Moisture : "
    );

    Serial.print(
      soilPercent
    );

    Serial.println(" %");


    Serial.print(
      "Light Value   : "
    );

    Serial.println(
      lightRaw
    );


    Serial.print(
      "Pump Status   : "
    );

    Serial.println(
      pumpState
      ? "ON"
      : "OFF"
    );

    // --------------------------------
    // MQTT PUBLISH
    // --------------------------------

    char tempString[10];
    char humidityString[10];
    char soilString[10];
    char lightString[10];

    dtostrf(
      data.temperature,
      1,
      1,
      tempString
    );

    dtostrf(
      data.humidity,
      1,
      1,
      humidityString
    );

    itoa(
      soilPercent,
      soilString,
      10
    );

    itoa(
      lightRaw,
      lightString,
      10
    );

    mqttClient.publish(
      TOPIC_TEMP,
      tempString
    );

    mqttClient.publish(
      TOPIC_HUMIDITY,
      humidityString
    );

    mqttClient.publish(
      TOPIC_SOIL,
      soilString
    );

    mqttClient.publish(
      TOPIC_LIGHT,
      lightString
    );

    mqttClient.publish(
      TOPIC_PUMP,
      pumpState
        ? "ON"
        : "OFF"
    );

    Serial.println(
      "MQTT Data     : PUBLISHED"
    );

    Serial.println(
      "--------------------------------"
    );
  }
}
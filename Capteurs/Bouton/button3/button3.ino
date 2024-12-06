#include <WiFi.h>
#include <MQTT.h>

// Informations WiFi et MQTT
const char ssid[] = "RobotiqueCPE";
const char pass[] = "AppareilLunaire:DauphinRadio";
const char mqtt_server[] = "134.214.51.148";

// Définition du bouton
const int buttonPin = 20; // GPIO pour le bouton
const char* topic = "button3"; // Topic pour publier l'état du bouton

// Variables pour gérer les changements d'état
bool previousState = HIGH;

WiFiClient net;
MQTTClient client;

void connectMQTT() {
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  while (!client.connect("ESP-Button3")) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nConnecté au broker MQTT.");
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, pass);
  client.begin(mqtt_server, net);

  pinMode(buttonPin, INPUT_PULLUP); // Bouton avec résistance pull-up interne
  connectMQTT();
}

void loop() {
  if (!client.connected()) {
    connectMQTT();
  }
  client.loop();

  bool currentState = digitalRead(buttonPin);
  if (currentState != previousState) {
    previousState = currentState;

    // Publier l'état du bouton
    String message = (currentState == LOW) ? "appuye" : "relache";
    client.publish(topic, message.c_str());

    Serial.printf("Bouton 3 : %s\n", message.c_str());
  }

  delay(50); // Anti-rebond
}
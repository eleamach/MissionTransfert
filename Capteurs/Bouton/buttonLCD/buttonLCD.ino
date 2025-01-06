#include <WiFi.h>
#include <PubSubClient.h>

// Informations WiFi et MQTT
const char* ssid = "RobotiqueCPE";
const char* password = "AppareilLunaire:DauphinRadio";
const char* mqtt_server = "134.214.51.148";
const char* topic = "/capteur/bouton/etat4LCD"; // Topic unique pour ce bouton

// Définition du bouton
const int buttonPin = 21; // GPIO pour le bouton
WiFiClient espClient;
PubSubClient client(espClient);

// Variables pour gérer l'état du bouton
bool previousState = HIGH;

void connectMQTT() {
  while (!client.connected()) {
    if (client.connect("Bouton4")) {
      Serial.println("Connecté au broker MQTT !");
    } else {
      Serial.print("Échec de connexion, état MQTT : ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  client.setServer(mqtt_server, 1883);

  pinMode(buttonPin, INPUT_PULLUP); // Résistance pull-up activée pour le bouton
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
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
    String message = (currentState == LOW) ? "pressed" : "released";
    client.publish(topic, message.c_str());

    Serial.printf("Bouton 4 : %s\n", message.c_str());
  }

  delay(50); // Anti-rebond
}
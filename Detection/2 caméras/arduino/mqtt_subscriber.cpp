#include <WiFi.h>
#include <PubSubClient.h>
#include "mqtt_subscriber.h"
#include "display.h"

// Paramètres MQTT : à adapter selon votre broker
const char* mqtt_server   = "134.214.51.148";
const int   mqtt_port     = 1883; 

// Topic de souscription
const char* subscribe_topic = "/detection/bi-camera/cam2-msg";

// Déclaration d'un client WiFi et d'un client MQTT
static WiFiClient espClient;
static PubSubClient mqttClient(espClient);

// Callback qui sera appelée à chaque message reçu
void mqttCallback(char* topic, byte* message, unsigned int length) {
  Serial.print("[MQTT] Message reçu sur le topic: ");
  Serial.println(topic);

  // Convertir le message binaire en String
  String messageTemp;
  for (int i = 0; i < length; i++) {
    messageTemp += (char)message[i];
  }
  Serial.print("[MQTT] Contenu: ");
  Serial.println(messageTemp);

  // Afficher le message via la fonction displayMessage(...)
  displayMessage(messageTemp.c_str());
}

// Fonction de connexion au broker MQTT
void reconnectMQTT() {
  // Tant qu’on n’est pas connecté au broker
  while (!mqttClient.connected()) {
    Serial.print("[MQTT] Tentative de connexion...");
    // Si la connexion est un succès
    if (mqttClient.connect("ESP32Client")) {
      Serial.println("connecté !");
      // On souscrit au topic souhaité
      mqttClient.subscribe(subscribe_topic);
    } else {
      Serial.print("[MQTT] Echec, code erreur= ");
      Serial.print(mqttClient.state());
      Serial.println(" Nouvelle tentative dans 5s...");
      delay(5000);
    }
  }
}

void setupMQTT() {
  // Configuration du client MQTT : serveur + callback
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(mqttCallback);
}

void handleMQTT() {
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop();
}
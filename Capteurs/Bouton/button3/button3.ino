#include <WiFi.h>
#include <MQTT.h>

// Informations WiFi et MQTT
const char* ssid = "VotreSSID"; 
const char* password = "VotrePassword"; 
const char* mqtt_server = "IP_SERVEUR_MQTT";  

// Pin du bouton
const int buttonPin = 19;  

// Identifiant unique pour ce bouton
const char* buttonId = "bouton1"; 

// Initialisation de la connexion WiFi et MQTT
WiFiClient net;
MQTTClient client;

// Variables pour détecter les changements d'état du bouton
bool previousState = HIGH;

void connectWiFi() {
  Serial.print("Connexion à ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connecté.");
  Serial.print("Adresse IP : ");
  Serial.println(WiFi.localIP());
}

void connectMQTT() {
  while (!client.connected()) {
    Serial.print("Connexion au broker MQTT...");
    if (client.connect(buttonId)) {
      Serial.println("Connecté au broker MQTT");
    } else {
      Serial.print("Échec, état : ");
      Serial.println(client.lastError());
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Configuration du bouton en entrée avec pull-up
  pinMode(buttonPin, INPUT_PULLUP);

  // Connexion WiFi
  connectWiFi();

  // Connexion au serveur MQTT
  client.begin(mqtt_server, net);
}

void loop() {
  // Reconnexion MQTT si nécessaire
  if (!client.connected()) {
    connectMQTT();
  }
  client.loop();

  // Lecture de l'état du bouton
  bool currentState = digitalRead(buttonPin);

  // Détection d'un changement d'état (appui ou relâchement)
  if (currentState != previousState) {
    previousState = currentState;

    // Construction du message MQTT
    String message = (currentState == LOW) ? "appuyé" : "relâché";
    Serial.print("Bouton ");
    Serial.print(buttonId);
    Serial.print(": ");
    Serial.println(message);

    // Publication de l'état du bouton sur un topic MQTT
    String topic = String("/atelier/") + buttonId;
    client.publish(topic.c_str(), message.c_str());
  }

  delay(50);  // Anti-rebond léger
}

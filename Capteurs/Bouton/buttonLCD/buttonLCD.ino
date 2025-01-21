#include <WiFi.h>
#include <PubSubClient.h>
#include <TFT_eSPI.h> // Bibliothèque pour l'écran TFT intégré

// Informations WiFi et MQTT
const char* ssid = "RobotiqueCPE";
const char* password = "AppareilLunaire:DauphinRadio";
const char* mqtt_server = "134.214.51.148";
const char* topic = "/capteur/bouton/etat4LCD"; // Topic unique pour ce bouton
const char* finishTopic = "/capteur/bouton/status"; // Topic pour l'état "finish"
const char* resetTopic = "/capteur/bouton/cmd"; // Topic pour l'état "reset"

// Définition du bouton
const int buttonPin = 21; // GPIO pour le bouton
WiFiClient espClient;
PubSubClient client(espClient);

// Configuration de l'écran TFT intégré
TFT_eSPI tft = TFT_eSPI(); // Initialisation de l'écran TFT

// Variables pour gérer l'état du bouton
bool previousState = HIGH;

void connectMQTT() {
  while (!client.connected()) {
    if (client.connect("Bouton4")) {
      Serial.println("Connecté au broker MQTT !");
      client.subscribe(finishTopic); // S'abonner au topic "finish"
      client.subscribe(resetTopic); // S'abonner au topic "cmd"
    } else {
      Serial.print("Échec de connexion, état MQTT : ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

// Callback pour gérer les messages MQTT reçus
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.printf("Message reçu : Topic=%s, Message=%s\n", topic, message.c_str());

  // Si le message est "finish", affichez "4" en jaune sur l'écran
  if (String(topic) == finishTopic && message == "finish") {
    tft.fillScreen(TFT_BLACK); // Efface l'écran
    tft.setTextColor(TFT_YELLOW, TFT_BLACK); // Texte jaune sur fond noir
    tft.setTextSize(5);        // Taille du texte
    tft.setCursor(50, 50);     // Positionne le texte au centre
    tft.print("8");            // Affiche "8"
  }
  if (String(topic) == resetTopic && message == "reset") {
    tft.fillScreen(TFT_BLACK); // Efface l'écran
    // Publier l'état du bouton
    String message = "waiting";
    client.publish(finishTopic, message.c_str());
  }
}

void setup() {
  Serial.begin(115200);

  // Initialisation de l'écran TFT
  tft.init();
  tft.setRotation(1); // Configure l'orientation de l'écran
  tft.fillScreen(TFT_BLACK); // Efface l'écran au démarrage

  pinMode(buttonPin, INPUT_PULLUP);

  // Initialisation WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnecté au Wi-Fi !");

  // Initialisation MQTT
  client.setServer(mqtt_server, 1883);
  client.setCallback(mqttCallback);
}

void loop() {
  if (!client.connected()) {
    connectMQTT();
  }
  client.loop();

  // Lecture de l'état du bouton
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
#include <WiFi.h>
#include <MQTT.h>
#include <TFT_eSPI.h>

// Informations WiFi et MQTT
const char ssid[] = "RobotiqueCPE";
const char pass[] = "AppareilLunaire:DauphinRadio";
const char mqtt_server[] = "134.214.51.148";
const char topic[] = "buttonCount"; // Topic pour recevoir le compteur
const char buttonTopic[] = "button4"; // Topic pour publier l'état du bouton

// Définition du bouton
const int buttonPin = 15; // GPIO pour le bouton

// Initialisation de l'écran
TFT_eSPI tft = TFT_eSPI();

WiFiClient net;
MQTTClient client;

// Variables pour gérer les changements d'état du bouton
bool previousState = HIGH;

void connectMQTT() {
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  while (!client.connect("ESP-Button4LCD")) {
    Serial.print(".");
    delay(1000);
  }

  client.subscribe(topic);
  Serial.println("\nConnecté au broker MQTT et abonné au topic.");
}

void messageReceived(String &topic, String &payload) {
  Serial.printf("Message reçu : %s\n", payload.c_str());

  // Extraction du compteur et de la couleur
  int separator = payload.indexOf(",");
  String count = payload.substring(0, separator);
  String color = payload.substring(separator + 1);

  // Mise à jour de l'écran
  tft.fillScreen(TFT_BLACK); // Efface l'écran
  tft.setCursor((tft.width() - 6 * 3 * 4) / 2, tft.height() / 2 - 30);
  tft.setTextSize(3);

  if (color == "green") {
    tft.setTextColor(TFT_GREEN);
  } else {
    tft.setTextColor(TFT_RED);
  }

  tft.printf("%s", count.c_str());
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, pass);
  client.begin(mqtt_server, net);
  client.onMessage(messageReceived);

  pinMode(buttonPin, INPUT_PULLUP); // Configuration du bouton
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);

  connectMQTT();
}

void loop() {
  // Gestion de la connexion MQTT
  if (!client.connected()) {
    connectMQTT();
  }
  client.loop();

  // Lecture de l'état du bouton
  bool currentState = digitalRead(buttonPin);
  if (currentState != previousState) {
    previousState = currentState;

    // Publier l'état du bouton
    String message = (currentState == LOW) ? "appuye" : "relache";
    client.publish(buttonTopic, message.c_str());

    Serial.printf("Bouton 4 : %s\n", message.c_str());
  }

  delay(50); // Anti-rebond
}
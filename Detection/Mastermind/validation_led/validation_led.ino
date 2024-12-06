#include <WiFi.h>
#include <MQTT.h>
#include <ArduinoJson.h>

// Déclaration des broches où sont connectées les LEDs
const int ledRedPin1 = 7; // LED sur la broche 7
const int ledRedPin2 = 0; // LED sur la broche 0
const int ledRedPin3 = 1; // LED sur la broche 1
const int ledRedPin4 = 8; // LED sur la broche 8

const int ledGreenPin1 = 23; // LED sur la broche 7
const int ledGreenPin2 = 22; // LED sur la broche 0
const int ledGreenPin3 = 21; // LED sur la broche 1
const int ledGreenPin4 = 20; // LED sur la broche 8

const char ssid[] = "RobotiqueCPE";
const char pass[] = "AppareilLunaire:DauphinRadio";


WiFiClient net;
MQTTClient client;

// Fonction pour faire clignoter les 4 LED
void blinkAllLeds(int times, int delayTime) {
  for (int i = 0; i < times; i++) {
    digitalWrite(ledRedPin1, HIGH);
    digitalWrite(ledRedPin2, HIGH);
    digitalWrite(ledRedPin3, HIGH);
    digitalWrite(ledRedPin4, HIGH);
    digitalWrite(ledGreenPin1, HIGH);
    digitalWrite(ledGreenPin2, HIGH);
    digitalWrite(ledGreenPin3, HIGH);
    digitalWrite(ledGreenPin4, HIGH);
    delay(delayTime);
    digitalWrite(ledRedPin1, LOW);
    digitalWrite(ledRedPin2, LOW);
    digitalWrite(ledRedPin3, LOW);
    digitalWrite(ledRedPin4, LOW);
    digitalWrite(ledGreenPin1, LOW);
    digitalWrite(ledGreenPin2, LOW);
    digitalWrite(ledGreenPin3, LOW);
    digitalWrite(ledGreenPin4, LOW);
    delay(delayTime);
  }
}

void lightUpRedLeds(int malPlace) {
  // Éteindre toutes les LEDs au départ
  digitalWrite(ledRedPin1, LOW);
  digitalWrite(ledRedPin2, LOW);
  digitalWrite(ledRedPin3, LOW);
  digitalWrite(ledRedPin4, LOW);

  // Allumer les LEDs nécessaires
  if (malPlace >= 1) digitalWrite(ledRedPin1, HIGH);
  if (malPlace >= 2) digitalWrite(ledRedPin2, HIGH);
  if (malPlace >= 3) digitalWrite(ledRedPin3, HIGH);
  if (malPlace >= 4) digitalWrite(ledRedPin4, HIGH);
}

void lightUpGreenLeds(int correcte) {
  // Éteindre toutes les LEDs au départ
  digitalWrite(ledGreenPin1, LOW);
  digitalWrite(ledGreenPin2, LOW);
  digitalWrite(ledGreenPin3, LOW);
  digitalWrite(ledGreenPin4, LOW);

  // Allumer les LEDs nécessaires
  if (correcte >= 1) digitalWrite(ledGreenPin1, HIGH);
  if (correcte >= 2) digitalWrite(ledGreenPin2, HIGH);
  if (correcte >= 3) digitalWrite(ledGreenPin3, HIGH);
  if (correcte >= 4) digitalWrite(ledGreenPin4, HIGH);
}

void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.print("\nconnecting...");
  while (!client.connect("arduinoClient", "public", "public")) {
    Serial.print("-");
    delay(1000);
  }

  Serial.println("\nconnected!");

  client.subscribe("/detection/mastermind/led");
}

void messageReceived(String &topic, String &payload) {
  Serial.println("Incoming message:");
  Serial.println("Topic: " + topic);
  Serial.println("Payload: " + payload);

  // Utilisation de la bibliothèque ArduinoJson pour analyser le message JSON
  StaticJsonDocument<200> doc;  // Crée un document JSON de taille fixe (ici 200 octets)
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.print("Failed to parse JSON: ");
    Serial.println(error.f_str());
    return;
  }

  // Récupérez les valeurs "correcte" et "mal_place"
  int correcte = doc["correcte"];
  int malPlace = doc["mal_place"];

  // Affichez les valeurs récupérées
  Serial.print("Correcte: ");
  Serial.println(correcte);
  Serial.print("Mal placé: ");
  Serial.println(malPlace);

   // Faire clignoter les LEDs 3 fois
  blinkAllLeds(3, 300); // 3 clignotements avec 300ms d'intervalle

  // Allumer un nombre de LEDs égal à `malPlace`
  lightUpRedLeds(malPlace);
  lightUpGreenLeds(correcte);
}




void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, pass);

  client.begin("134.214.51.148", net);
  client.onMessage(messageReceived);

  connect(); 
  
  // Initialisation des broches comme sorties
  pinMode(ledRedPin1, OUTPUT);
  pinMode(ledRedPin2, OUTPUT);
  pinMode(ledRedPin3, OUTPUT);
  pinMode(ledRedPin4, OUTPUT);

  // Initialisation des LEDs (allumées ou éteintes)
  digitalWrite(ledRedPin1, LOW); // LOW = éteint
  digitalWrite(ledRedPin2, LOW); 
  digitalWrite(ledRedPin3, LOW);
  digitalWrite(ledRedPin4, LOW);

  // Initialisation des broches comme sorties
  pinMode(ledGreenPin1, OUTPUT);
  pinMode(ledGreenPin2, OUTPUT);
  pinMode(ledGreenPin3, OUTPUT);
  pinMode(ledGreenPin4, OUTPUT);

  // Initialisation des LEDs (allumées ou éteintes)
  digitalWrite(ledGreenPin1, LOW); 
  digitalWrite(ledGreenPin2, LOW); 
  digitalWrite(ledGreenPin3, LOW);
  digitalWrite(ledGreenPin4, LOW);
}

void loop() {
  client.loop();
  delay(10); 

  if (!client.connected()) {
    connect();
  }
}

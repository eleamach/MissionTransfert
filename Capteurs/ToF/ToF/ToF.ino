#include <Wire.h>
#include <Adafruit_VL53L0X.h>
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Déclarations pour les capteurs ToF
Adafruit_VL53L0X lox1 = Adafruit_VL53L0X();
Adafruit_VL53L0X lox2 = Adafruit_VL53L0X();
Adafruit_VL53L0X lox3 = Adafruit_VL53L0X();

// Déclarations pour les LEDs RGB
#define LED_PIN1 8
#define LED_PIN2 10
#define NUM_LEDS 2
Adafruit_NeoPixel strip1(NUM_LEDS, LED_PIN1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2(NUM_LEDS, LED_PIN2, NEO_GRB + NEO_KHZ800);

// Broches XSHUT pour les capteurs
#define XSHUT1 6
#define XSHUT2 7
#define XSHUT3 5

// Plages de distance (en cm)
#define TOF_MIN 2.0f
#define TOF_MAX 120.0f
#define TARGET_DISTANCE1 30.0f
#define TARGET_DISTANCE2 30.0f
#define TARGET_DISTANCE3 30.0f
#define GREEN_TOLERANCE 4.0f

// Variables pour les états des capteurs
bool sensor1Green = false;
bool sensor2Green = false;
bool sensor3Green = false;
bool finishSent = false;

// Informations Wi-Fi et MQTT
const char* ssid = "RobotiqueCPE";
const char* password = "AppareilLunaire:DauphinRadio";
const char* mqtt_server = "10.50.127.10";
const char* mqtt_topic = "/capteur/tof/status";
WiFiClient espClient;
PubSubClient client(espClient);

// Initialisation des capteurs ToF
bool initializeToF(int xshutPin, Adafruit_VL53L0X &lox, uint8_t newAddress) {
  digitalWrite(xshutPin, HIGH); // Activer le capteur
  delay(10);

  if (!lox.begin(newAddress)) {
    Serial.print("Erreur : Capteur non initialisé à l'adresse 0x");
    Serial.println(newAddress, HEX);
    return false;
  }

  Serial.print("Capteur initialisé à l'adresse 0x");
  Serial.println(newAddress, HEX);
  return true;
}

// Configuration Wi-Fi
void setupWiFi() {
  Serial.println("Connexion au Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnecté au Wi-Fi !");
}

// Connexion MQTT
void reconnectMQTT() {
  while (!client.connected()) {
    Serial.println("Connexion au broker MQTT...");
    if (client.connect("ToFGameESP32")) {
      Serial.println("Connecté au broker MQTT !");
      if (client.subscribe("/capteur/tof/cmd")) {
        Serial.println("Souscription au topic /capteur/tof/cmd réussie.");
      }
    } else {
      Serial.print("Échec de connexion, état MQTT : ");
      Serial.print(client.state());
      Serial.println(". Nouvelle tentative dans 5 secondes...");
      delay(5000);
    }
  }
}

// Gestion des messages MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Message reçu sur le topic : ");
  Serial.print(topic);
  Serial.print(" -> ");
  Serial.println(message);

  if (String(topic) == "/capteur/tof/cmd" && message == "reset") {
    resetSystem();
  }
}

// Réinitialisation de l'écran
void resetSystem() {
  Serial.println("Réinitialisation de l'écran...");
  if (client.publish(mqtt_topic, "screen_reset")) {
    Serial.println("Message 'screen_reset' envoyé avec succès.");
  }
  finishSent = false; // Permet d'envoyer à nouveau "finish" après un reset
}

// Gérer chaque capteur
bool handleSensor(Adafruit_VL53L0X &lox, Adafruit_NeoPixel &strip, int ledIndex, int sensorNumber, float targetDistance) {
  VL53L0X_RangingMeasurementData_t measure;
  lox.rangingTest(&measure, false);

  if (measure.RangeStatus != 4) {
    float distance = measure.RangeMilliMeter / 10.0f; // Conversion en cm
    uint8_t red = 0, green = 0, blue = 0;
    calculateGradientColor(distance, targetDistance, red, green, blue);
    strip.setPixelColor(ledIndex, strip.Color(red, green, blue));
    strip.show();

    // Affichage simple des distances
    Serial.print("Capteur ");
    Serial.print(sensorNumber);
    Serial.print(" : Distance mesurée = ");
    Serial.print(distance, 2); // Affiche avec deux décimales
    Serial.print(" cm, Cible = ");
    Serial.print(targetDistance);
    Serial.println(" cm.");

    return fabs(distance - targetDistance) <= GREEN_TOLERANCE;
  } else {
    // Si le capteur est hors de portée
    Serial.print("Capteur ");
    Serial.print(sensorNumber);
    Serial.println(" : Hors de portée !");
    strip.setPixelColor(ledIndex, strip.Color(255, 0, 0)); // Rouge intense
    strip.show();
    return false;
  }
}

// Calculer la couleur dégradée
void calculateGradientColor(float distance, float targetDistance, uint8_t &red, uint8_t &green, uint8_t &blue) {
  float diff = fabs(distance - targetDistance);

  if (diff <= GREEN_TOLERANCE) {
    red = 0;
    green = 255;
    blue = 0;
  } else if (distance < targetDistance) {
    float normalizedDiff = (targetDistance - distance) / (targetDistance - TOF_MIN);
    normalizedDiff = min(max(normalizedDiff, 0.0f), 1.0f);
    blue = static_cast<uint8_t>(255 * (1.0f - normalizedDiff));
    red = static_cast<uint8_t>(255 * normalizedDiff);
    green = static_cast<uint8_t>((1.0f - normalizedDiff) * 127);
  } else {
    float normalizedDiff = (distance - targetDistance) / (TOF_MAX - targetDistance);
    normalizedDiff = min(max(normalizedDiff, 0.0f), 1.0f);
    blue = static_cast<uint8_t>(255 * normalizedDiff);
    red = static_cast<uint8_t>(255 * (1.0f - normalizedDiff));
    green = static_cast<uint8_t>((1.0f - normalizedDiff) * 127);
  }
}

// Configuration initiale
void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  pinMode(XSHUT1, OUTPUT);
  pinMode(XSHUT2, OUTPUT);
  pinMode(XSHUT3, OUTPUT);

  digitalWrite(XSHUT1, LOW);
  digitalWrite(XSHUT2, LOW);
  digitalWrite(XSHUT3, LOW);
  delay(10);

  initializeToF(XSHUT1, lox1, 0x30);
  initializeToF(XSHUT2, lox2, 0x31);
  initializeToF(XSHUT3, lox3, 0x32);

  strip1.begin();
  strip2.begin();
  strip1.setBrightness(20);
  strip2.setBrightness(20);
  strip1.show();
  strip2.show();

  setupWiFi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  reconnectMQTT();
}

// Boucle principale
void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  sensor1Green = handleSensor(lox1, strip1, 0, 1, TARGET_DISTANCE1); // Capteur 1
  sensor2Green = handleSensor(lox2, strip1, 1, 2, TARGET_DISTANCE2); // Capteur 2
  sensor3Green = handleSensor(lox3, strip2, 0, 3, TARGET_DISTANCE3); // Capteur 3

  if (sensor1Green && sensor2Green && sensor3Green) {
    if (!finishSent) {
      Serial.println("Tous les capteurs sont verts. Envoi du message FINISH.");
      client.publish(mqtt_topic, "finish");
      finishSent = true;
    }
  }

  delay(200);
}
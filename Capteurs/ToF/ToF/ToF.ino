#include <Wire.h>
#include <Adafruit_VL53L0X.h>
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <PubSubClient.h>

// === Déclarations des capteurs ToF (Time-of-Flight) ===
Adafruit_VL53L0X lox1 = Adafruit_VL53L0X();
Adafruit_VL53L0X lox2 = Adafruit_VL53L0X();
Adafruit_VL53L0X lox3 = Adafruit_VL53L0X();

// === Déclarations des bandeaux LED RGB ===
#define LED_PIN1 8    // Broche pour le premier bandeau
#define LED_PIN2 10   // Broche pour le second bandeau
#define NUM_LEDS 2    // Nombre de LEDs par bandeau
Adafruit_NeoPixel strip1(NUM_LEDS, LED_PIN1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2(NUM_LEDS, LED_PIN2, NEO_GRB + NEO_KHZ800);

// === Déclarations des broches XSHUT des capteurs ===
#define XSHUT1 6  // Broche XSHUT pour le capteur 1
#define XSHUT2 7  // Broche XSHUT pour le capteur 2
#define XSHUT3 5  // Broche XSHUT pour le capteur 3

// === Plages de distance et seuils ===
#define TOF_MIN 2.0f             // Distance minimale détectable par les capteurs (en cm)
#define TOF_MAX 120.0f           // Distance maximale détectable par les capteurs (en cm)
#define TARGET_DISTANCE1 30.0f   // Distance cible pour le capteur 1
#define TARGET_DISTANCE2 30.0f   // Distance cible pour le capteur 2
#define TARGET_DISTANCE3 30.0f   // Distance cible pour le capteur 3
#define GREEN_TOLERANCE 4.0f     // Tolérance pour le vert (distance idéale ± tolérance)

// === Variables d'état pour les capteurs ===
bool sensor1Green = false;  // État du capteur 1 (vert atteint)
bool sensor2Green = false;  // État du capteur 2 (vert atteint)
bool sensor3Green = false;  // État du capteur 3 (vert atteint)
bool finishSent = false;    // Indique si le message "finish" a déjà été envoyé

// === Informations pour la connexion Wi-Fi et MQTT ===
const char* ssid = "RobotiqueCPE";                // Nom du réseau Wi-Fi
const char* password = "AppareilLunaire:DauphinRadio"; // Mot de passe Wi-Fi
const char* mqtt_server = "10.50.127.10";         // Adresse du serveur MQTT
const char* mqtt_topic = "/capteur/tof/status";   // Topic MQTT pour publier les messages
WiFiClient espClient;
PubSubClient client(espClient); // Client MQTT

// === Initialisation des capteurs ToF ===
// Initialise un capteur ToF en configurant son adresse I2C
bool initializeToF(int xshutPin, Adafruit_VL53L0X &lox, uint8_t newAddress) {
  digitalWrite(xshutPin, HIGH); // Active le capteur
  delay(10);

  if (!lox.begin(newAddress)) { // Initialise avec une nouvelle adresse I2C
    Serial.print("Erreur : Capteur non initialisé à l'adresse 0x");
    Serial.println(newAddress, HEX);
    return false;
  }

  Serial.print("Capteur initialisé à l'adresse 0x");
  Serial.println(newAddress, HEX);
  return true;
}

// === Configuration du Wi-Fi ===
void setupWiFi() {
  Serial.println("Connexion au Wi-Fi...");
  WiFi.begin(ssid, password); // Se connecte au réseau Wi-Fi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnecté au Wi-Fi !");
}

// === Connexion au serveur MQTT ===
void reconnectMQTT() {
  while (!client.connected()) {
    Serial.println("Connexion au broker MQTT...");
    if (client.connect("ToFGameESP32")) { // Identifiant unique du client
      Serial.println("Connecté au broker MQTT !");
      if (client.subscribe("/capteur/tof/cmd")) { // Souscription au topic pour écouter les commandes
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

// === Gestion des messages MQTT ===
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

// === Réinitialisation de l'écran et des états ===
void resetSystem() {
  Serial.println("Réinitialisation de l'écran...");
  if (client.publish(mqtt_topic, "screen_reset")) {
    Serial.println("Message 'screen_reset' envoyé avec succès.");
  }
  finishSent = false; // Autorise l'envoi d'un nouveau "finish"
}

// === Gérer chaque capteur ToF ===
bool handleSensor(Adafruit_VL53L0X &lox, Adafruit_NeoPixel &strip, int ledIndex, int sensorNumber, float targetDistance) {
  VL53L0X_RangingMeasurementData_t measure;
  lox.rangingTest(&measure, false);

  if (measure.RangeStatus != 4) { // Vérifie si la mesure est valide
    float distance = measure.RangeMilliMeter / 10.0f; // Convertit en cm
    uint8_t red = 0, green = 0, blue = 0;
    calculateGradientColor(distance, targetDistance, red, green, blue);
    strip.setPixelColor(ledIndex, strip.Color(red, green, blue));
    strip.show();

    // Affiche les informations du capteur dans le Serial Monitor
    Serial.print("Capteur ");
    Serial.print(sensorNumber);
    Serial.print(" : Distance mesurée = ");
    Serial.print(distance, 2); // Deux décimales pour plus de précision
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

// === Calcul de la couleur dégradée pour une distance mesurée ===
void calculateGradientColor(float distance, float targetDistance, uint8_t &red, uint8_t &green, uint8_t &blue) {
  float diff = fabs(distance - targetDistance);

  if (diff <= GREEN_TOLERANCE) {
    red = 0;
    green = 255;
    blue = 0; // Vert si la distance est proche de la cible
  } else if (distance < targetDistance) {
    // Dégradé bleu → violet → rose si distance inférieure à la cible
    float normalizedDiff = (targetDistance - distance) / (targetDistance - TOF_MIN);
    normalizedDiff = min(max(normalizedDiff, 0.0f), 1.0f);
    blue = static_cast<uint8_t>(255 * (1.0f - normalizedDiff));
    red = static_cast<uint8_t>(255 * normalizedDiff);
    green = static_cast<uint8_t>((1.0f - normalizedDiff) * 127);
  } else {
    // Dégradé rose → rouge si distance supérieure à la cible
    float normalizedDiff = (distance - targetDistance) / (TOF_MAX - targetDistance);
    normalizedDiff = min(max(normalizedDiff, 0.0f), 1.0f);
    blue = static_cast<uint8_t>(255 * normalizedDiff);
    red = static_cast<uint8_t>(255 * (1.0f - normalizedDiff));
    green = static_cast<uint8_t>((1.0f - normalizedDiff) * 127);
  }
}

// === Configuration initiale ===
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

// === Boucle principale ===
void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  sensor1Green = handleSensor(lox1, strip1, 0, 1, TARGET_DISTANCE1);
  sensor2Green = handleSensor(lox2, strip1, 1, 2, TARGET_DISTANCE2);
  sensor3Green = handleSensor(lox3, strip2, 0, 3, TARGET_DISTANCE3);

  if (sensor1Green && sensor2Green && sensor3Green) {
    if (!finishSent) {
      Serial.println("Tous les capteurs sont verts. Envoi du message FINISH.");
      client.publish(mqtt_topic, "finish");
      finishSent = true;
    }
  }

  delay(200); // Pause entre les cycles de lecture
}
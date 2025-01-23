#include <Wire.h>
#include <Adafruit_VL53L0X.h>
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <PubSubClient.h>

// === Configuration du ruban LED ===
#define LED_PIN 10
#define NUM_LEDS 4
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRBW + NEO_KHZ800);

// === Configuration des capteurs ToF ===
#define TOF_MIN 2.0f
#define TOF_MAX 120.0f
#define GREEN_TOLERANCE 2.0f

bool gameFinished = false;  // Indique si le jeu est terminé
bool resetRequested = false;  // Indique si une commande reset a été reçue

// === Configuration MQTT ===
const char* ssid = "RobotiqueCPE";
const char* password = "AppareilLunaire:DauphinRadio";
const char* mqtt_server = "134.214.51.148";
const char* mqtt_topic_status = "/capteur/ToF/status";
const char* mqtt_topic_cmd = "/capteur/ToF/cmd";

WiFiClient espClient;
PubSubClient client(espClient);

// === Structure pour les broches XSHUT ===
struct TofConfig {
    const int xshutPin;
    const uint8_t address;
    const float targetDistance;    // Distance cible spécifique à chaque capteur
    Adafruit_VL53L0X sensor;
};

// === Configuration des capteurs ===
TofConfig tofs[] = {
    {5, 0x30, 20.0f, Adafruit_VL53L0X()},  // TOF1 - Target 20cm
    {4, 0x31, 30.0f, Adafruit_VL53L0X()},  // TOF2 - Target 30cm
    {7, 0x32, 40.0f, Adafruit_VL53L0X()},  // TOF3 - Target 40cm
    {6, 0x33, 25.0f, Adafruit_VL53L0X()}   // TOF4 - Target 25cm
};

// Connexion Wi-Fi
void setupWiFi() {
  Serial.println("Connexion au Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnecté au Wi-Fi !");
}

// Connexion au broker MQTT
void reconnectMQTT() {
  while (!client.connected()) {
    Serial.println("Connexion au broker MQTT...");
    if (client.connect("ToFGame")) { // ID unique pour le client
      Serial.println("Connecté au broker MQTT !");
      // Souscription au topic
      if (client.subscribe(mqtt_topic_cmd)) {
        Serial.println("Souscription au topic de commande réussie.");
      } else {
        Serial.println("Erreur lors de la souscription au topic.");
      }
    } else {
      Serial.print("Échec de connexion, état MQTT : ");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

// Callback MQTT pour traiter les messages reçus
void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Message reçu sur le topic : ");
  Serial.print(topic);
  Serial.print(" -> ");
  Serial.println(message);

  // Si le message est "reset"
  if (String(topic) == mqtt_topic_cmd && message == "reset") {
    Serial.println("Commande RESET reçue.");
    resetRequested = true;
  }
}

// Fonction pour gérer les capteurs ToF
float handleTofSensor(TofConfig& tof, int ledIndex) {
    VL53L0X_RangingMeasurementData_t measure;
    tof.sensor.rangingTest(&measure, false);
    float distance = 0;

    if (measure.RangeStatus != 4) {
        distance = measure.RangeMilliMeter / 10.0f;
        uint8_t red = 0, green = 0, blue = 0, white = 0;

        // Dégradé progressif basé sur la distance cible
        float maxDeviation = 15.0f;
        float deviation = abs(distance - tof.targetDistance);

        if (deviation <= maxDeviation) {
            float ratio = 1.0 - (deviation / maxDeviation);
            red = (uint8_t)(255 * (1.0 - ratio));
            green = (uint8_t)(255 * ratio);
        } else {
            red = 255;
            green = 0;
        }

        strip.setPixelColor(ledIndex, strip.Color(red, green, blue, white));
    } else {
        strip.setPixelColor(ledIndex, strip.Color(255, 0, 0, 0)); // Rouge si erreur
        distance = -1;
    }
    strip.show();
    return distance;
}

// Fonction pour gérer la victoire
void jeuGagne() {
    for (int i = 0; i < NUM_LEDS; i++) {
        strip.setPixelColor(i, strip.Color(0, 255, 0, 0));  // Vert
    }
    strip.show();
    Serial.println("Jeu gagné ! LEDs vertes.");
    client.publish(mqtt_topic_status, "finish");
    gameFinished = true;
}

// Fonction pour réinitialiser le jeu
void resetSystem() {
    gameFinished = false;
    resetRequested = false;
    Serial.println("Réinitialisation du jeu...");

    // Animation arc-en-ciel
    rainbow(20);

    // Réinitialisation des LEDs
    for (int i = 0; i < NUM_LEDS; i++) {
        strip.setPixelColor(i, strip.Color(0, 0, 0, 0));  // Éteindre toutes les LEDs
    }
    strip.show();

    // Envoi de l'état "waiting"
    client.publish(mqtt_topic_status, "waiting");
    Serial.println("Système en attente.");
}

// Fonction pour l'animation arc-en-ciel
void rainbow(int wait) {
    unsigned long startTime = millis();
    uint16_t hue = 0;
    while (millis() - startTime < 5000) {
        for (int i = 0; i < NUM_LEDS; i++) {
            strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(hue)));
        }
        strip.show();
        hue += 256;
        delay(wait);
    }
}

void setup() {
    Serial.begin(115200);
    Wire.begin(21, 22);

    // Initialisation des LEDs
    strip.begin();
    strip.setBrightness(50);
    strip.show();

    // Animation de démarrage
    rainbow(20);

    // Configuration Wi-Fi et MQTT
    setupWiFi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);

    // Initialisation des capteurs ToF
    for (auto& tof : tofs) {
        pinMode(tof.xshutPin, OUTPUT);
        digitalWrite(tof.xshutPin, LOW);
    }
    delay(300);

    for (auto& tof : tofs) {
        digitalWrite(tof.xshutPin, HIGH);
        delay(300);
        if (!tof.sensor.begin(tof.address)) {
            Serial.print("Erreur : Capteur non initialisé à l'adresse 0x");
            Serial.println(tof.address, HEX);
        }
    }

    Serial.println("\nTOF1\tTOF2\tTOF3\tTOF4");
    Serial.println("-------------------------");
}

void loop() {
    if (!client.connected()) {
        reconnectMQTT();
    }
    client.loop();

    if (resetRequested) {
        resetSystem();
        return;
    }

    if (gameFinished) {
        delay(500);
        return;
    }

    float distances[4];
    bool allInTarget = true;

    for (int i = 0; i < 4; i++) {
        distances[i] = handleTofSensor(tofs[i], i);
        if (distances[i] < tofs[i].targetDistance - GREEN_TOLERANCE || 
            distances[i] > tofs[i].targetDistance + GREEN_TOLERANCE) {
            allInTarget = false;
        }

        if (distances[i] >= 0) {
            Serial.print("TOF");
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(distances[i], 1);
            Serial.print("cm (target: ");
            Serial.print(tofs[i].targetDistance);
            Serial.print("cm)\t");
        } else {
            Serial.print("TOF");
            Serial.print(i + 1);
            Serial.print(": ERR\t");
        }
    }
    Serial.println();

    if (allInTarget) {
        jeuGagne();
    }

    delay(200);
}
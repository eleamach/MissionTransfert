#include <Wire.h>
#include <Adafruit_VL53L0X.h>
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <PubSubClient.h>

// === Déclarations des capteurs ToF (Time-of-Flight) ===
Adafruit_VL53L0X lox1 = Adafruit_VL53L0X();
Adafruit_VL53L0X lox2 = Adafruit_VL53L0X();
Adafruit_VL53L0X lox3 = Adafruit_VL53L0X();
Adafruit_VL53L0X lox4 = Adafruit_VL53L0X(); // Nouveau capteur ToF 4

// === Déclarations du bandeau LED RGB ===
#define LED_PIN 8       // Broche pour le bandeau LED
#define NUM_LEDS 4      // Nombre de LEDs total
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// === Déclarations des broches XSHUT des capteurs ===
#define XSHUT1 6  // Broche XSHUT pour le capteur 1
#define XSHUT2 7  // Broche XSHUT pour le capteur 2
#define XSHUT3 5  // Broche XSHUT pour le capteur 3
#define XSHUT4 4  // Broche XSHUT pour le capteur 4 (ajoutée)

// === Plages de distance et seuils ===
#define TOF_MIN 2.0f             // Distance minimale détectable par les capteurs (en cm)
#define TOF_MAX 120.0f           // Distance maximale détectable par les capteurs (en cm)
#define TARGET_DISTANCE1 30.0f   // Distance cible pour le capteur 1
#define TARGET_DISTANCE2 50.0f   // Distance cible pour le capteur 2
#define TARGET_DISTANCE3 70.0f   // Distance cible pour le capteur 3
#define TARGET_DISTANCE4 90.0f   // Distance cible pour le capteur 4
#define GREEN_TOLERANCE 2.0f     // Tolérance pour le vert (distance idéale ± tolérance)

// === Variables d'état pour les capteurs ===
bool sensor1Green = false;
bool sensor2Green = false;
bool sensor3Green = false;
bool sensor4Green = false; // Pour le capteur ToF 4
bool finishSent = false;

// === Informations pour la connexion Wi-Fi et MQTT ===
const char* ssid = "RobotiqueCPE";
const char* password = "AppareilLunaire:DauphinRadio";
const char* mqtt_server = "134.214.51.148";
const char* mqtt_topic = "/capteur/tof/status";
WiFiClient espClient;
PubSubClient client(espClient);

// === Initialisation des capteurs ToF ===
bool initializeToF(int xshutPin, Adafruit_VL53L0X &lox, uint8_t newAddress) {
  digitalWrite(xshutPin, HIGH); // Active le capteur
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

// === Réinitialisation des états ===
void resetSystem() {
  Serial.println("Réinitialisation de l'écran...");
  if (client.publish(mqtt_topic, "waiting")) {
    Serial.println("Message 'waiting' envoyé avec succès.");
  }
  finishSent = false; // Autorise l'envoi d'un nouveau "finish"
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
void fonction_tof1_degrade() {
  VL53L0X_RangingMeasurementData_t measure;
  lox1.rangingTest(&measure, false);

  if (measure.RangeStatus != 4) { // Vérifie si la mesure est valide
    float distance = measure.RangeMilliMeter / 10.0f; // Convertit en cm
    uint8_t red = 0, green = 0, blue = 0;

    // Gestion des plages de distance
    if (distance <= 15.0f) {
      // Rouge (0 cm à 15 cm)
      red = 255; green = 0; blue = 0;
    } else if (distance <= 20.0f) {
      // Orange (15 cm à 20 cm)
      red = 255; green = 165; blue = 0;
    } else if (distance <= 25.0f) {
      // Jaune (20 cm à 25 cm)
      red = 255; green = 255; blue = 0;
    } else if (distance <= 30.0f) {
      // Bleu (25 cm à 30 cm)
      red = 0; green = 0; blue = 127;
    } else if (distance <= 33.0f) {
      // Vert très clair (30 cm à 33 cm)
      red = 100; green = 230; blue = 100;
    } else if (distance <= 37.0f) {
      // Vert pétant (33 cm à 37 cm)
      red = 0; green = 255; blue = 0;
    } else if (distance <= 40.0f) {
      // Vert très clair (37 cm à 40 cm)
      red = 100; green = 230; blue = 100;
    } else if (distance <= 45.0f) {
      // Bleu (40 cm à 45 cm)
      red = 0; green = 0; blue = 127;
    } else if (distance <= 50.0f) {
      // Jaune (45 cm à 50 cm)
      red = 255; green = 255; blue = 0;
    } else if (distance <= 55.0f) {
      // Orange (50 cm à 55 cm)
      red = 255; green = 165; blue = 0;
    } else {
      // Rouge (au-delà de 55 cm)
      red = 255; green = 0; blue = 0;
    }

    // Mise à jour de la LED correspondante
    strip.setPixelColor(0, strip.Color(red, green, blue)); // LED 1 associée au ToF 1
    strip.show();

    // Affiche la distance pour le débogage
    Serial.print("TOF1 : Distance mesurée = ");
    Serial.print(distance, 2); // Deux décimales pour plus de précision
    Serial.println(" cm.");
  } else {
    // Capteur hors de portée (erreur de mesure)
    Serial.println("TOF1 : Hors de portée !");
    strip.setPixelColor(0, strip.Color(255, 0, 0)); // Rouge
    strip.show();
  }
}

void fonction_tof2_degrade() {
  VL53L0X_RangingMeasurementData_t measure;
  lox2.rangingTest(&measure, false);

  if (measure.RangeStatus != 4) { // Vérifie si la mesure est valide
    float distance = measure.RangeMilliMeter / 10.0f; // Convertit en cm
    uint8_t red = 0, green = 0, blue = 0;

    // Gestion des plages de distance
    if (distance <= 25.0f) {
      // Rouge (0 cm à 25 cm)
      red = 255; green = 0; blue = 0;
    } else if (distance <= 40.0f) {
      // Orange (25 cm à 40 cm)
      red = 255; green = 165; blue = 0;
    } else if (distance <= 45.0f) {
      // Jaune (40 cm à 45 cm)
      red = 255; green = 255; blue = 0;
    } else if (distance <= 50.0f) {
      // Bleu (45 cm à 50 cm)
      red = 0; green = 0; blue = 255;
    } else if (distance <= 53.0f) {
      // Vert très clair (50 cm à 53 cm)
      red = 100; green = 230; blue = 100;
    } else if (distance <= 57.0f) {
      // Vert pétant (53 cm à 57 cm)
      red = 0; green = 255; blue = 0;
    } else if (distance <= 60.0f) {
      // Vert très clair (57 cm à 60 cm)
      red = 100; green = 230; blue = 100;
    } else if (distance <= 65.0f) {
      // Bleu (60 cm à 65 cm)
      red = 0; green = 0; blue = 255;
    } else if (distance <= 70.0f) {
      // Jaune (65 cm à 70 cm)
      red = 255; green = 255; blue = 0;
    } else if (distance <= 75.0f) {
      // Orange (70 cm à 75 cm)
      red = 255; green = 165; blue = 0;
    } else {
      // Rouge (au-delà de 75 cm)
      red = 255; green = 0; blue = 0;
    }

    // Mise à jour de la LED correspondante
    strip.setPixelColor(1, strip.Color(red, green, blue)); // LED 2 associée au ToF 2
    strip.show();

    // Affiche la distance pour le débogage
    Serial.print("TOF2 : Distance mesurée = ");
    Serial.print(distance, 2); // Deux décimales pour plus de précision
    Serial.println(" cm.");
  } else {
    // Capteur hors de portée (erreur de mesure)
    Serial.println("TOF2 : Hors de portée !");
    strip.setPixelColor(1, strip.Color(255, 0, 0)); // Rouge
    strip.show();
  }
}

void fonction_tof3_degrade() {
  VL53L0X_RangingMeasurementData_t measure;
  lox3.rangingTest(&measure, false);

  if (measure.RangeStatus != 4) { // Vérifie si la mesure est valide
    float distance = measure.RangeMilliMeter / 10.0f; // Convertit en cm
    uint8_t red = 0, green = 0, blue = 0;

    // Gestion des plages de distance
    if (distance <= 45.0f) {
      // Rouge (0 cm à 45 cm)
      red = 255; green = 0; blue = 0;
    } else if (distance <= 60.0f) {
      // Orange (45 cm à 60 cm)
      red = 255; green = 165; blue = 0;
    } else if (distance <= 65.0f) {
      // Jaune (60 cm à 65 cm)
      red = 255; green = 255; blue = 0;
    } else if (distance <= 70.0f) {
      // Bleu (65 cm à 70 cm)
      red = 0; green = 0; blue = 255;
    } else if (distance <= 73.0f) {
      // Vert très clair (70 cm à 73 cm)
      red = 100; green = 230; blue = 100;
    } else if (distance <= 77.0f) {
      // Vert pétant (73 cm à 77 cm)
      red = 0; green = 255; blue = 0;
    } else if (distance <= 80.0f) {
      // Vert très clair (77 cm à 80 cm)
      red = 100; green = 230; blue = 100;
    } else if (distance <= 85.0f) {
      // Bleu (80 cm à 85 cm)
      red = 0; green = 0; blue = 255;
    } else if (distance <= 90.0f) {
      // Jaune (85 cm à 90 cm)
      red = 255; green = 255; blue = 0;
    } else if (distance <= 95.0f) {
      // Orange (90 cm à 95 cm)
      red = 255; green = 165; blue = 0;
    } else {
      // Rouge (au-delà de 95 cm)
      red = 255; green = 0; blue = 0;
    }

    // Mise à jour de la LED correspondante
    strip.setPixelColor(2, strip.Color(red, green, blue)); // LED 3 associée au ToF 3
    strip.show();

    // Affiche la distance pour le débogage
    Serial.print("TOF3 : Distance mesurée = ");
    Serial.print(distance, 2); // Deux décimales pour plus de précision
    Serial.println(" cm.");
  } else {
    // Capteur hors de portée (erreur de mesure)
    Serial.println("TOF3 : Hors de portée !");
    strip.setPixelColor(2, strip.Color(255, 0, 0)); // Rouge
    strip.show();
  }
}

// === Configuration initiale ===
void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  pinMode(XSHUT1, OUTPUT);
  pinMode(XSHUT2, OUTPUT);
  pinMode(XSHUT3, OUTPUT);
  pinMode(XSHUT4, OUTPUT);

  digitalWrite(XSHUT1, LOW);
  digitalWrite(XSHUT2, LOW);
  digitalWrite(XSHUT3, LOW);
  digitalWrite(XSHUT4, LOW);
  delay(10);

  initializeToF(XSHUT1, lox1, 0x30);
  initializeToF(XSHUT2, lox2, 0x31);
  initializeToF(XSHUT3, lox3, 0x32);
  initializeToF(XSHUT4, lox4, 0x33);

  strip.begin();
  strip.setBrightness(10);
  strip.show();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnecté au Wi-Fi !");
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

// === Boucle principale ===
void loop() {
  if (!client.connected()) {
    if (client.connect("ToFGameESP32")) {
      client.subscribe("/capteur/tof/cmd");
    }
  }
  client.loop();

  // Appel de la fonction pour gérer TOF1 et la LED correspondante
  fonction_tof1_degrade();
  fonction_tof2_degrade();
  fonction_tof3_degrade();

  // Suppression des appels à handleSensor pour éviter l'erreur
  // Les fonctions spécifiques pour les autres TOF seront ajoutées plus tard
  // sensor2Green = handleSensor(lox2, 1, 2, TARGET_DISTANCE2);
  // sensor3Green = handleSensor(lox3, 2, 3, TARGET_DISTANCE3);
  // sensor4Green = handleSensor(lox4, 3, 4, TARGET_DISTANCE4);

  delay(1000); // Pause entre les cycles
}
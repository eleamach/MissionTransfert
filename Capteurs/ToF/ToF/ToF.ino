#include <Wire.h>
#include <Adafruit_VL53L0X.h>
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <PubSubClient.h>

// === Déclarations des capteurs ToF (Time-of-Flight) ===
Adafruit_VL53L0X lox1 = Adafruit_VL53L0X();
Adafruit_VL53L0X lox2 = Adafruit_VL53L0X();
Adafruit_VL53L0X lox3 = Adafruit_VL53L0X();

// === Déclarations des rubans LED RGB ===
#define LED_PIN_RUB1 8  // Broche pour le ruban LED 1
#define LED_PIN_RUB2 10 // Broche pour le ruban LED 2
#define LED_PIN_RUB3 11 // Broche pour le ruban LED 3
#define NUM_LEDS 1      // Nombre de LEDs par ruban
Adafruit_NeoPixel strip1(NUM_LEDS, LED_PIN_RUB1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2(NUM_LEDS, LED_PIN_RUB2, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip3(NUM_LEDS, LED_PIN_RUB3, NEO_GRB + NEO_KHZ800);

// === Déclarations des broches XSHUT des capteurs ===
#define XSHUT1 6  // Broche XSHUT pour le capteur 1
#define XSHUT2 7  // Broche XSHUT pour le capteur 2
#define XSHUT3 5

// === Plages de distance et seuils ===
#define TOF_MIN 2.0f             // Distance minimale détectable par les capteurs (en cm)
#define TOF_MAX 120.0f           // Distance maximale détectable par les capteurs (en cm)
#define TARGET_DISTANCE1 30.0f   // Distance cible pour le capteur 1
#define TARGET_DISTANCE2 30.0f   // Distance cible pour le capteur 2
#define TARGET_DISTANCE3 30.0f
#define GREEN_TOLERANCE 4.0f     // Tolérance pour le vert (distance idéale ± tolérance)

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

// === Fonction de dégradé pour le capteur 1 ===
void fonction_tof1_degrade() {
  VL53L0X_RangingMeasurementData_t measure;
  lox1.rangingTest(&measure, false);

  if (measure.RangeStatus != 4) { // Vérifie si la mesure est valide
    float distance = measure.RangeMilliMeter / 10.0f; // Convertit en cm
    uint8_t red = 0, green = 0, blue = 0;

    // Gestion des plages de distance
    if (distance <= 15.0f) {
      red = 255; green = 0; blue = 0; // Rouge
    } else if (distance <= 20.0f) {
      red = 255; green = 165; blue = 0; // Orange
    } else if (distance <= 25.0f) {
      red = 255; green = 255; blue = 0; // Jaune
    } else if (distance <= 33.0f) {
      red = 100; green = 230; blue = 100; // Vert très clair
    } else if (distance <= 37.0f) {
      red = 0; green = 255; blue = 0; // Vert pétant
    } else {
      red = 255; green = 0; blue = 0; // Rouge au-delà
    }

    strip1.setPixelColor(0, strip1.Color(red, green, blue)); // LED du ruban 1
    strip1.show();
    Serial.print("TOF1 : Distance mesurée = ");
    Serial.print(distance, 2); // Deux décimales
    Serial.println(" cm.");
  } else {
    Serial.println("TOF1 : Hors de portée !");
    strip1.setPixelColor(0, strip1.Color(255, 0, 0)); // Rouge
    strip1.show();
  }
}

// === Fonction de dégradé pour le capteur 2 ===
void fonction_tof2_degrade() {
  VL53L0X_RangingMeasurementData_t measure;
  lox2.rangingTest(&measure, false);

  if (measure.RangeStatus != 4) { // Vérifie si la mesure est valide
    float distance = measure.RangeMilliMeter / 10.0f; // Convertit en cm
    uint8_t red = 0, green = 0, blue = 0;

    // Gestion des plages de distance
    if (distance <= 15.0f) {
      red = 255; green = 0; blue = 0; // Rouge
    } else if (distance <= 20.0f) {
      red = 255; green = 165; blue = 0; // Orange
    } else if (distance <= 25.0f) {
      red = 255; green = 255; blue = 0; // Jaune
    } else if (distance <= 33.0f) {
      red = 100; green = 230; blue = 100; // Vert très clair
    } else if (distance <= 37.0f) {
      red = 0; green = 255; blue = 0; // Vert pétant
    } else {
      red = 255; green = 0; blue = 0; // Rouge au-delà
    }

    strip2.setPixelColor(0, strip2.Color(red, green, blue)); // LED du ruban 2
    strip2.show();
    Serial.print("TOF2 : Distance mesurée = ");
    Serial.print(distance, 2); // Deux décimales
    Serial.println(" cm.");
  } else {
    Serial.println("TOF2 : Hors de portée !");
    strip2.setPixelColor(0, strip2.Color(255, 0, 0)); // Rouge
    strip2.show();
  }
}

void fonction_tof3_degrade() {
  VL53L0X_RangingMeasurementData_t measure;
  lox3.rangingTest(&measure, false);

  if (measure.RangeStatus != 4) { // Vérifie si la mesure est valide
    float distance = measure.RangeMilliMeter / 10.0f; // Convertit en cm
    uint8_t red = 0, green = 0, blue = 0;

    // Gestion des plages de distance
    if (distance <= 15.0f) {
      red = 255; green = 0; blue = 0; // Rouge
    } else if (distance <= 20.0f) {
      red = 255; green = 165; blue = 0; // Orange
    } else if (distance <= 25.0f) {
      red = 255; green = 255; blue = 0; // Jaune
    } else if (distance <= 33.0f) {
      red = 100; green = 230; blue = 100; // Vert très clair
    } else if (distance <= 37.0f) {
      red = 0; green = 255; blue = 0; // Vert pétant
    } else {
      red = 255; green = 0; blue = 0; // Rouge au-delà
    }

    strip3.setPixelColor(0, strip3.Color(red, green, blue)); // LED du ruban 2
    strip3.show();
    Serial.print("TOF3 : Distance mesurée = ");
    Serial.print(distance, 2); // Deux décimales
    Serial.println(" cm.");
  } else {
    Serial.println("TOF3 : Hors de portée !");
    strip3.setPixelColor(0, strip3.Color(255, 0, 0)); // Rouge
    strip3.show();
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
  initializeToF(XSHUT3, lox2, 0x32);

  strip1.begin();
  strip1.setBrightness(50);
  strip1.show();

  strip2.begin();
  strip2.setBrightness(50);
  strip2.show();

  strip3.begin();
  strip3.setBrightness(50);
  strip3.show();
}

// === Boucle principale ===
void loop() {
  fonction_tof1_degrade();
  fonction_tof2_degrade();
  fonction_tof3_degrade();
  delay(1000); // Pause entre les cycles
}
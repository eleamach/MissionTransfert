#include <Wire.h>
#include <Adafruit_VL53L0X.h>
#include <Adafruit_NeoPixel.h>

// Déclarations pour les capteurs ToF
Adafruit_VL53L0X lox1 = Adafruit_VL53L0X();
Adafruit_VL53L0X lox2 = Adafruit_VL53L0X();
Adafruit_VL53L0X lox3 = Adafruit_VL53L0X();

// Déclarations pour les LEDs RGB
#define LED_PIN1 8
#define LED_PIN2 10
#define NUM_LEDS 2 // Deux LEDs par bandeau
Adafruit_NeoPixel strip1(NUM_LEDS, LED_PIN1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2(NUM_LEDS, LED_PIN2, NEO_GRB + NEO_KHZ800);

// Broches XSHUT pour les capteurs
#define XSHUT1 6
#define XSHUT2 7
#define XSHUT3 5

// Plages de distance (en cm)
#define TOF_MIN 2.0f            // Distance minimale mesurable
#define TOF_MAX 120.0f          // Distance maximale mesurable
#define TARGET_DISTANCE1 35.0f  // Distance cible pour le capteur 1
#define TARGET_DISTANCE2 55.0f  // Distance cible pour le capteur 2
#define TARGET_DISTANCE3 12.0f  // Distance cible pour le capteur 3

// Tolérance pour le vert (distance cible atteinte)
#define GREEN_TOLERANCE 0.5f

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22); // SDA = GPIO21, SCL = GPIO22

  // Initialisation des broches XSHUT
  pinMode(XSHUT1, OUTPUT);
  pinMode(XSHUT2, OUTPUT);
  pinMode(XSHUT3, OUTPUT);

  // Désactiver les capteurs au départ
  digitalWrite(XSHUT1, LOW);
  digitalWrite(XSHUT2, LOW);
  digitalWrite(XSHUT3, LOW);
  delay(10);

  // Initialisation des capteurs
  if (!initializeToF(XSHUT1, lox1, 0x30)) while (1);
  if (!initializeToF(XSHUT2, lox2, 0x31)) while (1);
  if (!initializeToF(XSHUT3, lox3, 0x32)) while (1);

  // Initialisation des bandeaux LEDs
  strip1.begin();
  strip2.begin();
  strip1.setBrightness(20); // Luminosité réduite
  strip2.setBrightness(20);
  strip1.show();
  strip2.show();

  Serial.println("Configuration terminée !");
}

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

void loop() {
  Serial.println("Lecture des capteurs...");

  // Lecture et gestion des capteurs
  handleSensor(lox1, strip1, 0, TARGET_DISTANCE1, true);  // Capteur 1 → Bandeau 1, LED 0
  handleSensor(lox2, strip1, 1, TARGET_DISTANCE2, true); // Capteur 2 → Bandeau 1, LED 1
  handleSensor(lox3, strip2, 0, TARGET_DISTANCE3, true); // Capteur 3 → Bandeau 2, LED 0

  delay(200);
}

// Gérer chaque capteur séparément
void handleSensor(Adafruit_VL53L0X &lox, Adafruit_NeoPixel &strip, int ledIndex, float targetDistance, bool includeFarBlue) {
  VL53L0X_RangingMeasurementData_t measure;
  lox.rangingTest(&measure, false);

  if (measure.RangeStatus != 4) {
    float distance = measure.RangeMilliMeter / 10.0f; // Conversion en cm
    Serial.print("Capteur ");
    Serial.print(ledIndex + 1);
    Serial.print(" : ");
    Serial.print(distance);
    Serial.println(" cm");

    uint8_t red = 0, green = 0, blue = 0;
    calculateGradientColor(distance, targetDistance, includeFarBlue, red, green, blue);
    strip.setPixelColor(ledIndex, strip.Color(red, green, blue));
  } else {
    Serial.print("Capteur ");
    Serial.print(ledIndex + 1);
    Serial.println(" : Hors de portée !");
    strip.setPixelColor(ledIndex, strip.Color(255, 0, 0)); // Rouge intense
  }

  strip.show();
}

// Calculer la couleur dégradée
void calculateGradientColor(float distance, float targetDistance, bool includeFarBlue, uint8_t &red, uint8_t &green, uint8_t &blue) {
  float diff = fabs(distance - targetDistance);

  if (diff <= GREEN_TOLERANCE) {
    // Vert : distance correcte
    red = 0;
    green = 255;
    blue = 0;
  } else if (distance < targetDistance) {
    // Dégradé bleu → violet → rose pour les distances inférieures
    float normalizedDiff = (targetDistance - distance) / (targetDistance - TOF_MIN);
    normalizedDiff = min(max(normalizedDiff, 0.0f), 1.0f);
    blue = static_cast<uint8_t>(255 * (1.0f - normalizedDiff));
    red = static_cast<uint8_t>(255 * normalizedDiff);
    green = static_cast<uint8_t>((1.0f - normalizedDiff) * 127);
  } else {
    if (includeFarBlue) {
      // Dégradé rose → bleu pour les distances supérieures
      float normalizedDiff = (distance - targetDistance) / (TOF_MAX - targetDistance);
      normalizedDiff = min(max(normalizedDiff, 0.0f), 1.0f);
      blue = static_cast<uint8_t>(255 * normalizedDiff);
      red = static_cast<uint8_t>(255 * (1.0f - normalizedDiff));
      green = static_cast<uint8_t>((1.0f - normalizedDiff) * 127);
    } else {
      // Dégradé rose → rouge pour les distances supérieures
      float normalizedDiff = (distance - targetDistance) / (TOF_MAX - targetDistance);
      normalizedDiff = min(max(normalizedDiff, 0.0f), 1.0f);
      red = 255;
      blue = static_cast<uint8_t>(255 * (1.0f - normalizedDiff));
      green = static_cast<uint8_t>((1.0f - normalizedDiff) * 127);
    }
  }
}
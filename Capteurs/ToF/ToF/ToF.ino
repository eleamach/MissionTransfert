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
#define TARGET_DISTANCE 30.0f
#define GREEN_TOLERANCE 2.0f

// === Structure pour les broches XSHUT ===
struct TofConfig {
    const int xshutPin;
    const uint8_t address;
    const float targetDistance;    // Distance cible spécifique à chaque capteur
    Adafruit_VL53L0X sensor;
};

// === Configuration des capteurs ===
TofConfig tofs[] = {
    {6, 0x30, 20.0f, Adafruit_VL53L0X()},  // TOF1 - Target 20cm
    {7, 0x31, 30.0f, Adafruit_VL53L0X()},  // TOF2 - Target 30cm
    {5, 0x32, 40.0f, Adafruit_VL53L0X()},  // TOF3 - Target 40cm
    {4, 0x33, 25.0f, Adafruit_VL53L0X()}   // TOF4 - Target 25cm
};

// === Fonction unique pour gérer un capteur ToF ===
float handleTofSensor(TofConfig& tof, int ledIndex) {
    VL53L0X_RangingMeasurementData_t measure;
    tof.sensor.rangingTest(&measure, false);
    float distance = 0;

    if (measure.RangeStatus != 4) {
        distance = measure.RangeMilliMeter / 10.0f;
        uint8_t red = 0, green = 0, blue = 0, white = 0;

        // Calcul du dégradé progressif avec la distance cible spécifique
        float maxDeviation = 15.0f;    // Écart maximum pour le dégradé
        float deviation = abs(distance - tof.targetDistance);  // Utilise la target spécifique
        
        if (deviation <= maxDeviation) {
            // Calcul de l'intensité (0.0 à 1.0)
            float ratio = 1.0 - (deviation / maxDeviation);
            
            // Plus proche de la cible = plus vert
            // Plus loin de la cible = plus rouge
            red = (uint8_t)(255 * (1.0 - ratio));
            green = (uint8_t)(255 * ratio);
        } else {
            // Au-delà de l'écart maximum = rouge complet
            red = 255;
            green = 0;
        }

        strip.setPixelColor(ledIndex, strip.Color(red, green, blue, white));
    } else {
        strip.setPixelColor(ledIndex, strip.Color(255, 0, 0, 0));
        distance = -1;
    }
    
    strip.show();
    return distance;
}

// === Ajout de la fonction arc-en-ciel ===
void rainbow(int wait) {
    unsigned long startTime = millis();
    uint16_t hue = 0;
    
    while (millis() - startTime < 5000) { // Pendant 5 secondes
        for(int i=0; i<NUM_LEDS; i++) {
            strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(hue)));
        }
        strip.show();
        
        hue += 256;  // Déplace la teinte pour l'animation
        delay(wait);
    }
}

void setup() {
    Serial.begin(115200);
    Wire.begin(21, 22);
    
    // Initialisation du ruban LED
    strip.begin();
    strip.setBrightness(50);
    strip.show();
    
    // Animation arc-en-ciel pendant 5 secondes
    rainbow(20);  // 20ms entre chaque changement de couleur
    
    // Initialisation des broches XSHUT
    for (auto& tof : tofs) {
        pinMode(tof.xshutPin, OUTPUT);
        digitalWrite(tof.xshutPin, LOW);
    }
    delay(300);

    // Initialisation séquentielle des capteurs
    for (auto& tof : tofs) {
        digitalWrite(tof.xshutPin, HIGH);
        delay(300);
        
        if (!tof.sensor.begin(tof.address)) {
            Serial.print("Erreur : Capteur non initialisé à l'adresse 0x");
            Serial.println(tof.address, HEX);
        }
    }

    // En-tête du tableau
    Serial.println("\nTOF1\tTOF2\tTOF3\tTOF4");
    Serial.println("-------------------------");
}

void loop() {
    float distances[4];
    
    // Lecture séquentielle des capteurs
    for (int i = 0; i < 4; i++) {
        distances[i] = handleTofSensor(tofs[i], i);
        
        // Affichage détaillé pour chaque capteur
        if (distances[i] >= 0) {
            Serial.print("TOF");
            Serial.print(i+1);
            Serial.print(": ");
            Serial.print(distances[i], 1);
            Serial.print("cm (target: ");
            Serial.print(tofs[i].targetDistance);
            Serial.print("cm)\t");
        } else {
            Serial.print("TOF");
            Serial.print(i+1);
            Serial.print(": ERR\t");
        }
    }
    Serial.println();
    delay(200);
}
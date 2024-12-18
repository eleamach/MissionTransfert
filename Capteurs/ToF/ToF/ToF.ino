#include <Wire.h>
#include <Adafruit_VL53L0X.h>

// Création des capteurs
Adafruit_VL53L0X lox1 = Adafruit_VL53L0X();
Adafruit_VL53L0X lox2 = Adafruit_VL53L0X();
Adafruit_VL53L0X lox3 = Adafruit_VL53L0X();
Adafruit_VL53L0X lox4 = Adafruit_VL53L0X();

// Définition des broches XSHUT
#define XSHUT1 6
#define XSHUT2 7
#define XSHUT3 5
#define XSHUT4 4

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22); // SDA = GPIO21, SCL = GPIO22

  // Initialisation des GPIO XSHUTs
  pinMode(XSHUT1, OUTPUT);
  pinMode(XSHUT2, OUTPUT);
  pinMode(XSHUT3, OUTPUT);
  pinMode(XSHUT4, OUTPUT);

  // Désactiver tous les capteurs au départ
  digitalWrite(XSHUT1, LOW);
  digitalWrite(XSHUT2, LOW);
  digitalWrite(XSHUT3, LOW);
  digitalWrite(XSHUT4, LOW);
  delay(10);

  // Initialisation des capteurs
  if (initializeToF(XSHUT1, lox1, 0x30)) {
    Serial.println("Capteur 1 initialisé à l'adresse 0x30");
  } else {
    Serial.println("Erreur : Capteur 1 non initialisé !");
    while (1);
  }

  if (initializeToF(XSHUT2, lox2, 0x31)) {
    Serial.println("Capteur 2 initialisé à l'adresse 0x31");
  } else {
    Serial.println("Erreur : Capteur 2 non initialisé !");
    while (1);
  }

  if (initializeToF(XSHUT3, lox3, 0x32)) {
    Serial.println("Capteur 3 initialisé à l'adresse 0x32");
  } else {
    Serial.println("Erreur : Capteur 3 non initialisé !");
    while (1);
  }

  if (initializeToF(XSHUT4, lox4, 0x33)) {
    Serial.println("Capteur 4 initialisé à l'adresse 0x33");
  } else {
    Serial.println("Erreur : Capteur 4 non initialisé !");
    while (1);
  }

  Serial.println("Tous les capteurs sont prêts !");
}

bool initializeToF(int xshutPin, Adafruit_VL53L0X &lox, uint8_t newAddress) {
  digitalWrite(xshutPin, HIGH); // Activer le capteur
  delay(10);

  if (!lox.begin(newAddress)) {
    Serial.print("Erreur d'initialisation du capteur à l'adresse 0x");
    Serial.println(newAddress, HEX);
    return false;
  }
  return true;
}

void loop() {
  VL53L0X_RangingMeasurementData_t measure;

  // Lecture du capteur 1
  lox1.rangingTest(&measure, false);
  Serial.print("Capteur 1 (0x30) : ");
  printDistance(measure);

  // Lecture du capteur 2
  lox2.rangingTest(&measure, false);
  Serial.print("Capteur 2 (0x31) : ");
  printDistance(measure);

  // Lecture du capteur 3
  lox3.rangingTest(&measure, false);
  Serial.print("Capteur 3 (0x32) : ");
  printDistance(measure);

  // Lecture du capteur 4
  lox4.rangingTest(&measure, false);
  Serial.print("Capteur 4 (0x33) : ");
  printDistance(measure);

  delay(500); // Pause entre les lectures
}

void printDistance(VL53L0X_RangingMeasurementData_t measure) {
  if (measure.RangeStatus != 4) { // 4 signifie hors de portée
    Serial.print(measure.RangeMilliMeter / 10.0);
    Serial.println(" cm");
  } else {
    Serial.println("Hors de portée !");
  }
}
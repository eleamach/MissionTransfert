#include <SPI.h>

// Configuration des broches
#define PIN_SCK  21   // Pin pour SCK
#define PIN_CS   18   // Pin pour CS
#define PIN_MISO 20   // Pin pour MISO
#define PIN_MOSI 19   // Pin pour MOSI

// Buffers pour les LEDs et les boutons
uint8_t red[16] = {0};
uint8_t green[16] = {0};
uint8_t blue[16] = {0};
bool buttons[16] = {false};

// Variables pour éviter le spam
unsigned long lastPressTime[16] = {0}; // Dernier temps de traitement pour chaque bouton
const unsigned long debounceDelay = 1000; // Délai d'anti-spam en millisecondes

// Initialisation des broches SPI
void setupSPI() {
  pinMode(PIN_SCK, OUTPUT);
  pinMode(PIN_CS, OUTPUT);
  pinMode(PIN_MISO, OUTPUT);
  pinMode(PIN_MOSI, INPUT);

  digitalWrite(PIN_SCK, HIGH); // SCK inactif haut
  digitalWrite(PIN_CS, HIGH);  // CS inactif haut
  digitalWrite(PIN_MISO, LOW); // MISO inactif bas
}

// Écriture d'une trame sur le bus SPI
void writeFrame(const uint8_t* frame) {
  for (int i = 0; i < 128; i++) { // 16 octets x 8 bits = 128 bits
    digitalWrite(PIN_SCK, LOW);
    delayMicroseconds(5);

    // Envoie le bit courant
    if (frame[i / 8] & (1 << (7 - (i % 8)))) {
      digitalWrite(PIN_MISO, HIGH);
    } else {
      digitalWrite(PIN_MISO, LOW);
    }
    delayMicroseconds(5);

    digitalWrite(PIN_SCK, HIGH);
    delayMicroseconds(10);
  }
}

// Lecture d'une trame depuis le bus SPI
void readFrame(uint8_t* frame) {
  for (int i = 0; i < 128; i++) { // 16 octets x 8 bits = 128 bits
    digitalWrite(PIN_SCK, LOW);
    delayMicroseconds(5);

    // Lecture du bit courant
    if (digitalRead(PIN_MOSI)) {
      frame[i / 8] |= (1 << (7 - (i % 8)));
    } else {
      frame[i / 8] &= ~(1 << (7 - (i % 8)));
    }

    digitalWrite(PIN_SCK, HIGH);
    delayMicroseconds(10);
  }
}

// Envoi de l'état actuel au contrôleur des LEDs
void commit() {
  uint8_t buttonsBuffer[16] = {0};

  digitalWrite(PIN_CS, HIGH); // Active le contrôleur SPI
  delayMicroseconds(15);

  writeFrame(red);   // Écrit les données rouges
  writeFrame(green); // Écrit les données vertes
  writeFrame(blue);  // Écrit les données bleues

  readFrame(buttonsBuffer); // Lit les boutons
  for (int i = 0; i < 16; i++) {
    buttons[i] = (buttonsBuffer[i] == 0x00); // Bouton pressé si 0x00
  }

  digitalWrite(PIN_CS, LOW); // Désactive le contrôleur SPI
  delayMicroseconds(500);
}

// Fonction pour éteindre toutes les LEDs
void blackout() {
  memset(red, 0, sizeof(red));   // Éteint toutes les LEDs rouges
  memset(green, 0, sizeof(green)); // Éteint toutes les LEDs vertes
  memset(blue, 0, sizeof(blue));  // Éteint toutes les LEDs bleues
  commit(); // Met à jour l'état des LEDs
}

// Active une couleur aléatoire pour une LED
void setRandomColorForButton(int buttonIndex) {
  blackout(); // Éteint toutes les LEDs avant d'activer une nouvelle
  if (buttonIndex >= 0 && buttonIndex < 16) {
    int randomColor = random(4); // Génère un entier entre 0 et 3
    if (randomColor == 0) {
      red[buttonIndex] = 255;   // Rouge
    } else if (randomColor == 1) {
      green[buttonIndex] = 255; // Vert
    } else if (randomColor == 2) {
      blue[buttonIndex] = 255;  // Bleu
    } else if (randomColor == 3) {
      red[buttonIndex] = 255;   // Jaune (Rouge + Vert)
      green[buttonIndex] = 255;
    }
  }
  commit(); // Envoie l'état au contrôleur
}

// Affiche l'état des boutons
void printButtons() {
  for (int i = 0; i < 16; i++) {
    Serial.print("Button ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(buttons[i] ? "Pressed" : "Released");
  }
}

void setup() {
  Serial.begin(115200);
  setupSPI();

  // Initialisation
  Serial.println("Test des LEDs avec couleurs aléatoires (rouge, vert, bleu, jaune).");
  blackout(); // Éteint toutes les LEDs au démarrage

  randomSeed(analogRead(0)); // Initialisation de la génération aléatoire
}

void loop() {
  commit(); // Met à jour les boutons et LEDs
  unsigned long currentTime = millis();

  for (int i = 0; i < 16; i++) {
    if (buttons[i]) { // Si un bouton est pressé
      if (currentTime - lastPressTime[i] > debounceDelay) { // Vérifie le délai d'anti-spam
        Serial.print("Button pressed: ");
        Serial.println(i);

        setRandomColorForButton(i); // Active une couleur aléatoire
        delay(500);                 // Garde la LED allumée pendant 0,5 seconde
        blackout();                 // Éteint la LED après 0,5 seconde

        lastPressTime[i] = currentTime; // Met à jour le temps du dernier appui
      }
    }
  }

  delay(50); // Pause légère pour lisser le comportement
}
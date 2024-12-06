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

// Variables pour la gestion des séquences
int sequencePositions[5] = {0};       // Positions des LEDs pour la séquence
uint8_t sequenceColors[5][3] = {0};   // Couleurs associées à chaque position
int currentStep = 0;                  // Étape actuelle de la séquence
unsigned long lastUpdateTime = 0;     // Dernière mise à jour de la séquence
const unsigned long updateInterval = 500; // Intervalle entre chaque étape (en ms)
bool sequenceActive = true;           // Indique si une séquence est en cours

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
  memset(red, 0, sizeof(red));
  memset(green, 0, sizeof(green));
  memset(blue, 0, sizeof(blue));
  commit();
}

// Génère une séquence aléatoire avec des couleurs fixes (rouge, vert, bleu)
void generateSequence() {
  blackout(); // Éteint toutes les LEDs

  for (int i = 0; i < 5; i++) {
    sequencePositions[i] = random(16); // Position aléatoire entre 0 et 15

    // Couleurs fixes attribuées de manière aléatoire
    int randomColorIndex = random(3); // Choix entre 0 (Rouge), 1 (Vert), 2 (Bleu)
    if (randomColorIndex == 0) {
      sequenceColors[i][0] = 255; // Rouge
      sequenceColors[i][1] = 0;   // Pas de Vert
      sequenceColors[i][2] = 0;   // Pas de Bleu
    } else if (randomColorIndex == 1) {
      sequenceColors[i][0] = 0;   // Pas de Rouge
      sequenceColors[i][1] = 255; // Vert
      sequenceColors[i][2] = 0;   // Pas de Bleu
    } else if (randomColorIndex == 2) {
      sequenceColors[i][0] = 0;   // Pas de Rouge
      sequenceColors[i][1] = 0;   // Pas de Vert
      sequenceColors[i][2] = 255; // Bleu
    }
  }

  // Affiche la séquence générée pour le débogage
  Serial.println("Nouvelle séquence générée !");
  for (int i = 0; i < 5; i++) {
    Serial.print("Étape ");
    Serial.print(i);
    Serial.print(": LED ");
    Serial.print(sequencePositions[i]);
    Serial.print(" Couleur ");
    if (sequenceColors[i][0] == 255) {
      Serial.println("Rouge");
    } else if (sequenceColors[i][1] == 255) {
      Serial.println("Vert");
    } else if (sequenceColors[i][2] == 255) {
      Serial.println("Bleu");
    }
  }
}
// Affiche l'étape actuelle de la séquence
void processSequence() {
  if (currentStep < 5) {
    blackout(); // Éteint toutes les LEDs avant d'activer la suivante
    int position = sequencePositions[currentStep];
    red[position] = sequenceColors[currentStep][0];
    green[position] = sequenceColors[currentStep][1];
    blue[position] = sequenceColors[currentStep][2];
    commit(); // Met à jour les LEDs
    delay(500); // Maintient la LED allumée pendant 500 ms
    blackout();
    currentStep++;
  } else {
    currentStep = 0; // Réinitialise la séquence
    sequenceActive = false; // Termine la séquence
  }
}

void setup() {
  Serial.begin(115200);
  setupSPI();

  Serial.println("Initialisation...");
  blackout(); // Éteint toutes les LEDs au démarrage

  randomSeed(analogRead(0)); // Initialise le générateur aléatoire
  generateSequence(); // Crée une première séquence
}

void loop() {
  commit(); // Met à jour les boutons et LEDs

  if (sequenceActive) {
    unsigned long currentTime = millis();
    if (currentTime - lastUpdateTime >= updateInterval) {
      lastUpdateTime = currentTime;
      processSequence(); // Affiche l'étape suivante de la séquence
    }
  }
}
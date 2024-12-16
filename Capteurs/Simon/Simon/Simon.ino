#include <SPI.h>

// Configuration des broches
#define PIN_SCK  21
#define PIN_CS   18
#define PIN_MISO 20
#define PIN_MOSI 19

// Buffers pour les LEDs et les boutons
uint8_t red[16] = {0};
uint8_t green[16] = {0};
uint8_t blue[16] = {0};
bool buttons[16] = {false};

// Variables pour la gestion des séquences
int currentSequenceSize = 1;          // Taille actuelle de la séquence
int maxSequenceSize = 5;              // Taille maximale pour le niveau actuel
int sequencePositions[8] = {0};       // Positions des LEDs pour la séquence (jusqu'à 8 LEDs)
uint8_t sequenceColors[8][3] = {0};   // Couleurs associées à chaque position
int currentStep = 0;                  // Étape actuelle de la séquence
unsigned long lastUpdateTime = 0;     // Dernière mise à jour de la séquence
const unsigned long updateInterval = 500; // Intervalle entre chaque étape (en ms)
bool sequenceActive = true;           // Indique si une séquence est en cours
bool userInputMode = false;           // Mode utilisateur actif
int userInputIndex = 0;               // Index de l'étape utilisateur en cours
bool levelComplete = false;           // Indique si le niveau est terminé
int currentLevel = 1;                 // Niveau actuel

// Initialisation des broches SPI
void setupSPI() {
  pinMode(PIN_SCK, OUTPUT);
  pinMode(PIN_CS, OUTPUT);
  pinMode(PIN_MISO, OUTPUT);
  pinMode(PIN_MOSI, INPUT);

  digitalWrite(PIN_SCK, HIGH);
  digitalWrite(PIN_CS, HIGH);
  digitalWrite(PIN_MISO, LOW);
}

// Écriture d'une trame sur le bus SPI
void writeFrame(const uint8_t* frame) {
  for (int i = 0; i < 128; i++) {
    digitalWrite(PIN_SCK, LOW);
    delayMicroseconds(5);
    digitalWrite(PIN_MISO, (frame[i / 8] & (1 << (7 - (i % 8)))) ? HIGH : LOW);
    delayMicroseconds(5);
    digitalWrite(PIN_SCK, HIGH);
    delayMicroseconds(10);
  }
}

// Lecture d'une trame depuis le bus SPI
void readFrame(uint8_t* frame) {
  for (int i = 0; i < 128; i++) {
    digitalWrite(PIN_SCK, LOW);
    delayMicroseconds(5);
    if (digitalRead(PIN_MOSI)) frame[i / 8] |= (1 << (7 - (i % 8)));
    else frame[i / 8] &= ~(1 << (7 - (i % 8)));
    digitalWrite(PIN_SCK, HIGH);
    delayMicroseconds(10);
  }
}

// Envoi de l'état actuel au contrôleur des LEDs
void commit() {
  uint8_t buttonsBuffer[16] = {0};

  digitalWrite(PIN_CS, HIGH);
  delayMicroseconds(15);

  writeFrame(red);
  writeFrame(green);
  writeFrame(blue);

  readFrame(buttonsBuffer);
  for (int i = 0; i < 16; i++) {
    buttons[i] = (buttonsBuffer[i] == 0x00); // Bouton pressé si 0x00
  }

  digitalWrite(PIN_CS, LOW);
  delayMicroseconds(500);
}

// Fonction pour éteindre toutes les LEDs
void blackout() {
  memset(red, 0, sizeof(red));
  memset(green, 0, sizeof(green));
  memset(blue, 0, sizeof(blue));
  commit();
}

// Génère une séquence aléatoire (les nouvelles étapes ajoutent une LED)
void generateSequence() {
  blackout();
  if (currentSequenceSize == 1) {
    // Génère une séquence entière au début
    for (int i = 0; i < maxSequenceSize; i++) {
      sequencePositions[i] = random(16);
      int randomColorIndex = random(3);
      sequenceColors[i][0] = (randomColorIndex == 0) ? 255 : 0;
      sequenceColors[i][1] = (randomColorIndex == 1) ? 255 : 0;
      sequenceColors[i][2] = (randomColorIndex == 2) ? 255 : 0;
    }
  }
  Serial.println("Nouvelle étape ajoutée !");
}

// Affiche la séquence actuelle
void processSequence() {
  if (currentStep < currentSequenceSize) {
    blackout();
    int position = sequencePositions[currentStep];
    red[position] = sequenceColors[currentStep][0];
    green[position] = sequenceColors[currentStep][1];
    blue[position] = sequenceColors[currentStep][2];
    commit();
    delay(500); // Temps pour voir la LED
    blackout();
    delay(300); // Pause entre les étapes
    currentStep++;
  } else {
    currentStep = 0;
    sequenceActive = false;
    userInputMode = true; // Active le mode utilisateur
    Serial.println("Reproduisez la séquence !");
  }
}

// Vérifie les entrées utilisateur
void checkUserInput() {
  for (int i = 0; i < 16; i++) {
    if (buttons[i]) {
      blackout();
      delay(100);
      if (i == sequencePositions[userInputIndex]) {
        red[i] = sequenceColors[userInputIndex][0];
        green[i] = sequenceColors[userInputIndex][1];
        blue[i] = sequenceColors[userInputIndex][2];
        commit();
        delay(300);
        userInputIndex++;
        if (userInputIndex >= currentSequenceSize) {
          Serial.println("Étape réussie !");
          for (int j = 0; j < 3; j++) {
            blackout();
            memset(green, 255, sizeof(green)); // LEDs en vert
            commit();
            delay(200);
            blackout();
            commit();
            delay(200);
          }
          if (currentSequenceSize < maxSequenceSize) {
            currentSequenceSize++; // Augmente la taille de la séquence
          } else if (currentLevel == 1) {
            Serial.println("Niveau 1 terminé, passage au niveau 2 !");
            currentLevel = 2;
            currentSequenceSize = 1; // Réinitialise à 1 LED
            maxSequenceSize = 8;     // Passe à 8 LEDs
          } else if (currentLevel == 2) {
            Serial.println("Niveau 2 terminé !");
            levelComplete = true;
          }
          resetGame();
        }
      } else {
        Serial.println("Mauvaise touche !");
        for (int j = 0; j < 3; j++) {
          blackout();
          memset(red, 255, sizeof(red)); // LEDs en rouge
          commit();
          delay(200);
          blackout();
          commit();
          delay(200);
        }
        resetGame();
      }
    }
  }
}

// Réinitialise le jeu
void resetGame() {
  userInputMode = false;
  sequenceActive = true;
  userInputIndex = 0;
  currentStep = 0;
  generateSequence();
}

void setup() {
  Serial.begin(115200);
  setupSPI();
  blackout();
  randomSeed(analogRead(0));
  generateSequence();
}

void loop() {
  commit();
  if (sequenceActive && !levelComplete) {
    unsigned long currentTime = millis();
    if (currentTime - lastUpdateTime >= updateInterval) {
      lastUpdateTime = currentTime;
      processSequence();
    }
  }
  if (userInputMode) {
    checkUserInput();
  }
}
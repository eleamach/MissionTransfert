#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Configuration des broches
#define PIN_SCK  21
#define PIN_CS   18
#define PIN_MISO 20
#define PIN_MOSI 19

// GPIOs pour les LEDs de progression
#define PIN_LED1  4
#define PIN_LED2  5
#define PIN_LED3  6

// Buffers pour les LEDs et les boutons
uint8_t red[16] = {0};
uint8_t green[16] = {0};
uint8_t blue[16] = {0};
bool buttons[16] = {false};

// Variables pour la gestion des séquences
int currentSequenceSize = 1;           // Taille actuelle de la séquence
int maxSequenceSize = 5;               // Taille maximale pour le niveau actuel
int sequencePositions[10] = {0};       // Positions des LEDs pour la séquence (jusqu'à 10 LEDs)
uint8_t sequenceColors[10][3] = {0};   // Couleurs associées à chaque position
int currentStep = 0;                   // Étape actuelle de la séquence
unsigned long lastUpdateTime = 0;      // Dernière mise à jour de la séquence
const unsigned long updateInterval = 500; // Intervalle entre chaque étape (en ms)
bool sequenceActive = true;            // Indique si une séquence est en cours
bool userInputMode = false;            // Mode utilisateur actif
int userInputIndex = 0;                // Index de l'étape utilisateur en cours
bool levelComplete = false;            // Indique si le niveau est terminé
int currentLevel = 1;                  // Niveau actuel
bool levelTransition = false; // Empêche la mise à jour multiple des niveaux

// Informations Wi-Fi et MQTT
const char* ssid = "RobotiqueCPE";
const char* password = "AppareilLunaire:DauphinRadio";
const char* mqtt_server = "134.214.51.148";
const char* mqtt_topic = "capteur/simon/status";

WiFiClient espClient;
PubSubClient client(espClient);

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
    if (client.connect("SimonGameESP32")) { // ID unique pour le client
      Serial.println("Connecté au broker MQTT !");
    } else {
      Serial.print("Échec, état MQTT : ");
      Serial.print(client.state());
      Serial.println(". Nouvelle tentative dans 5 secondes...");
      delay(5000);
    }
  }
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

void updateProgressionLEDs() {
  digitalWrite(PIN_LED1, currentLevel >= 2 ? HIGH : LOW);
  digitalWrite(PIN_LED2, currentLevel >= 3 ? HIGH : LOW);
  digitalWrite(PIN_LED3, levelComplete ? HIGH : LOW);

  // Ajouter une pause pour stabiliser la transition
  delay(200); // Délai en millisecondes
}

// Génère une séquence aléatoire
void generateSequence() {
  blackout();
  if (currentSequenceSize == 1) {
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
    delay(500);
    blackout();
    delay(300);
    currentStep++;
  } else {
    currentStep = 0;
    sequenceActive = false;
    userInputMode = true;
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
        // Bonne entrée : allume la LED correspondante
        red[i] = sequenceColors[userInputIndex][0];
        green[i] = sequenceColors[userInputIndex][1];
        blue[i] = sequenceColors[userInputIndex][2];
        commit();
        delay(300);
        userInputIndex++;

        // Vérifie si l'étape courante est terminée
        if (userInputIndex >= currentSequenceSize) {
          Serial.println("Étape réussie !");
          for (int j = 0; j < 3; j++) {
            blackout();
            memset(green, 255, sizeof(green)); // LEDs en vert (succès)
            commit();
            delay(200);
            blackout();
            commit();
            delay(200);
          }

          // Progression dans la séquence ou niveau
          if (currentSequenceSize < maxSequenceSize) {
            currentSequenceSize++;
          } else if (!levelTransition) { // Vérifie que la transition n'a pas eu lieu
            levelTransition = true; // Active le verrou pour empêcher plusieurs transitions
            if (currentLevel == 1) {
              Serial.println("Niveau 1 terminé !");
              currentLevel = 2;
              currentSequenceSize = 1;
              maxSequenceSize = 8; // Configuration pour le niveau 2
              updateProgressionLEDs();
            } else if (currentLevel == 2) {
              Serial.println("Niveau 2 terminé !");
              currentLevel = 3;
              currentSequenceSize = 1;
              maxSequenceSize = 10; // Configuration pour le niveau 3
              updateProgressionLEDs();
            } else if (currentLevel == 3) {
              Serial.println("Niveau 3 terminé !");
              levelComplete = true;

              // Publie un message MQTT pour signaler la fin du jeu
              client.publish(mqtt_topic, "finish");
              updateProgressionLEDs();
            }
          }
          resetGame(); // Réinitialise pour le prochain niveau ou la prochaine séquence
        }
      } else {
        // Mauvaise entrée : réinitialise la séquence courante
        Serial.println("Mauvaise touche !");
        for (int j = 0; j < 3; j++) {
          blackout();
          memset(red, 255, sizeof(red)); // LEDs en rouge (échec)
          commit();
          delay(200);
          blackout();
          commit();
          delay(200);
        }
        resetGame(); // Réinitialise la séquence
      }
    }
  }
}

void resetGame() {
  userInputMode = false;
  sequenceActive = true;
  userInputIndex = 0;
  currentStep = 0;
  levelTransition = false; // Réinitialise le verrou pour permettre une nouvelle progression
  generateSequence();
}

void setup() {
  Serial.begin(115200);
  setupSPI();

  // Configurer Wi-Fi et MQTT
  setupWiFi();
  client.setServer(mqtt_server, 1883);

  pinMode(PIN_LED1, OUTPUT);
  pinMode(PIN_LED2, OUTPUT);
  pinMode(PIN_LED3, OUTPUT);
  
  digitalWrite(PIN_LED1, LOW);
  digitalWrite(PIN_LED2, LOW);
  digitalWrite(PIN_LED3, LOW);

  blackout();
  randomSeed(analogRead(0));
  generateSequence();
}

void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop(); // Gestion des événements MQTT

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
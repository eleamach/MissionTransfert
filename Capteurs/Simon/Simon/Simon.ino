#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Arduino.h>

// Configuration des broches
#define PIN_SCK  21
#define PIN_CS   18
#define PIN_MISO 20
#define PIN_MOSI 19

// GPIOs pour les LEDs de progression
#define PIN_LED1  4
#define PIN_LED2  5
#define PIN_LED3  6

// GPIO pour l'audio
const int AudioPin = 3;

// Notes de musique (en Hz)
const int NOTE_C4 = 261;  // Do grave
const int NOTE_D4 = 294;  // Ré
const int NOTE_Eb4 = 311; // Mi bémol
const int NOTE_F4 = 349;  // Fa
const int NOTE_G4 = 392;  // Sol
const int NOTE_Ab4 = 415; // La bémol
const int NOTE_Bb4 = 466; // Si bémol
const int NOTE_C5 = 523;  // Do
const int NOTE_D5 = 587;  // Ré
const int NOTE_Eb5 = 622; // Mi bémol
const int NOTE_F5 = 698;  // Fa
const int NOTE_G5 = 784;  // Sol
const int NOTE_Ab5 = 830; // La bémol
const int NOTE_Bb5 = 932; // Si bémol
const int NOTE_C6 = 1047; // Do aigu
const int NOTE_D6 = 1175; // Ré aigu

// Mapping des 16 boutons vers des notes fixes
const int buttonToNoteMap[16] = {
  NOTE_C4, NOTE_D4, NOTE_Eb4, NOTE_F4,
  NOTE_G4, NOTE_Ab4, NOTE_Bb4, NOTE_C5,
  NOTE_D5, NOTE_Eb5, NOTE_F5, NOTE_G5,
  NOTE_Ab5, NOTE_Bb5, NOTE_C6, NOTE_D6
};

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
const char* mqtt_topic = "/capteur/simon/status";

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

void tone(int pin, int freq, int duration) {
    analogWriteFrequency(pin, freq);
    analogWrite(pin, 128);
    delay(duration);          // Attend la durée spécifiée
    analogWrite(pin, 0);      // Éteint la note
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
      // Souscrit au topic après connexion
      if (client.subscribe("/capteur/simon/cmd")) {
        Serial.println("Souscription au topic /capteur/simon/cmd réussie.");
      } else {
        Serial.println("Erreur lors de la souscription au topic /capteur/simon/cmd.");
      }
    } else {
      Serial.print("Échec de connexion, état MQTT : ");
      Serial.print(client.state());
      Serial.println(". Nouvelle tentative dans 5 secondes...");
      delay(5000);
    }
  }
}

// Fonction callback pour traiter les messages MQTT reçus
void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  Serial.print("Message reçu sur le topic : ");
  Serial.print(topic);
  Serial.print(" -> ");
  Serial.println(message);

  // Vérifie si le message est "reset"
  if (String(topic) == "/capteur/simon/cmd" && message == "reset") {
    Serial.println("Commande RESET reçue. Réinitialisation en cours...");
    resetSystem();
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

void playNoteForButton(int buttonIndex) {
  if (buttonIndex >= 0 && buttonIndex < 16) {
    int note = buttonToNoteMap[buttonIndex];
    tone(AudioPin, note, 100); // Joue la note pendant 300ms
    delay(100);                // Pause pour la durée du son
    noTone(AudioPin);          // Arrête le son
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

void showLevelCompleteLEDs(bool isSuccess) {
  blackout(); // Éteint toutes les LEDs

  if (isSuccess) {
    // LEDs vertes pour succès
    memset(green, 255, sizeof(green));
  } else {
    // LEDs rouges pour échec
    memset(red, 255, sizeof(red));
  }

  commit();
  delay(200); // Affiche pendant 300 ms
  blackout(); // Éteint les LEDs après affichage
  delay(200);
}

void updateProgressionLEDs() {
  digitalWrite(PIN_LED1, currentLevel >= 2 ? HIGH : LOW);
  digitalWrite(PIN_LED2, currentLevel >= 3 ? HIGH : LOW);
  digitalWrite(PIN_LED3, levelComplete ? HIGH : LOW);

  // Ajouter une pause pour stabiliser la transition
  delay(100); // Délai en millisecondes
}

// Génère une séquence aléatoire
void generateSequence() {
  blackout();
  
  // Liste des couleurs primaires et secondaires
  const uint8_t colors[6][3] = {
    {255, 0, 0},   // Rouge
    {0, 255, 0},   // Vert
    {0, 0, 255},   // Bleu
    {0, 255, 255}, // Cyan
    {255, 0, 255}, // Magenta
    {255, 255, 0}  // Jaune
  };

  if (currentSequenceSize == 1) {
    for (int i = 0; i < maxSequenceSize; i++) {
      sequencePositions[i] = random(16); // Position aléatoire sur le pad
      int colorIndex = random(6);        // Index aléatoire pour choisir une couleur
      
      // Assigne les couleurs primaires/secondaires
      sequenceColors[i][0] = colors[colorIndex][0]; // Rouge
      sequenceColors[i][1] = colors[colorIndex][1]; // Vert
      sequenceColors[i][2] = colors[colorIndex][2]; // Bleu
    }
  }
  Serial.println("Nouvelle étape ajoutée !");
}

void processSequence() {
  if (currentStep < currentSequenceSize) {
    blackout();
    delay(200);
    int position = sequencePositions[currentStep];
    red[position] = sequenceColors[currentStep][0];
    green[position] = sequenceColors[currentStep][1];
    blue[position] = sequenceColors[currentStep][2];
    commit();

    // Jouer le son correspondant à la LED affichée
    int note = buttonToNoteMap[position]; // Récupère la note associée au bouton
    tone(AudioPin, note, 100);            // Joue la note pendant 100ms
    delay(50);                           // Pause pour la durée du son
    noTone(AudioPin);                     // Arrête le son

    blackout();
    currentStep++;
  } else {
    currentStep = 0;
    sequenceActive = false;
    userInputMode = true;
    Serial.println("Reproduisez la séquence !");
  }
}

void displayNumberOne() {
  // Éteint toutes les LEDs
  blackout();

  // Active les LEDs nécessaires pour afficher le chiffre "1" en blanc
  red[13] = 255; green[13] = 255; blue[13] = 0;  // Deuxième ligne, première colonne
  red[7]  = 255; green[7]  = 255; blue[7]  = 0;  // Quatrième ligne, troisième colonne
  red[8]  = 255; green[8]  = 255; blue[8]  = 0;  // Première ligne, deuxième colonne
  red[9]  = 255; green[9]  = 255; blue[9]  = 0;  // Deuxième ligne, deuxième colonne
  red[10]  = 255; green[10]  = 255; blue[10]  = 0;  // Troisième ligne, deuxième colonne
  red[11] = 255; green[11] = 255; blue[11] = 0;  // Quatrième ligne, deuxième colonne
  red[15] = 255; green[15] = 255; blue[15] = 0;  // Quatrième ligne, première colonne

  // Met à jour les LEDs
  commit();
}

void playSuccessMelody() {
    // Mélodie de succès : Do, Mi, Sol, Do aigu
    tone(AudioPin, buttonToNoteMap[0], 150);  // Do (C4)
    tone(AudioPin, buttonToNoteMap[2], 150);  // Mi (E4)
    tone(AudioPin, buttonToNoteMap[4], 150);  // Sol (G4)
    tone(AudioPin, buttonToNoteMap[7], 300);  // Do aigu (C5)
    delay(300);
    noTone(AudioPin);  // Assure que le son est arrêté
}

void playFailureMelody() {
    // Mélodie d'échec : Sol aigu, Mi, Do, Sol grave
    tone(AudioPin, buttonToNoteMap[4], 100);  // Sol (G4)
    tone(AudioPin, buttonToNoteMap[2], 100);  // Mi (E4)
    tone(AudioPin, buttonToNoteMap[0], 100);  // Do (C4)
    tone(AudioPin, buttonToNoteMap[8], 300);  // Sol grave (G3)
    delay(300);
    noTone(AudioPin);  // Assure que le son est arrêté
}


// Vérifie les entrées utilisateur
unsigned long userInputLastTime = 0; // Stocke le temps du dernier appui utilisateur
const unsigned long inputTimeout = 10000; // Délai d'attente en millisecondes (8 secondes)

void checkUserInput() {
  bool inputReceived = false; // Flag pour savoir si une touche est pressée

  for (int i = 0; i < 16; i++) {
    if (buttons[i]) {
      inputReceived = true; // Une touche a été pressée
      userInputLastTime = millis(); // Réinitialise le timer

      // Allume la lumière associée immédiatement
      red[i] = sequenceColors[userInputIndex][0];
      green[i] = sequenceColors[userInputIndex][1];
      blue[i] = sequenceColors[userInputIndex][2];
      commit();

      // Jouer le son correspondant après avoir affiché la lumière
      playNoteForButton(i);

      // Éteint la lumière immédiatement après avoir joué le son
      red[i] = 0;
      green[i] = 0;
      blue[i] = 0;
      commit();

      delay(10); // Très courte pause pour stabiliser le traitement

      if (i == sequencePositions[userInputIndex]) {
        // Bonne entrée
        userInputIndex++;

        // Vérifie si l'étape courante est terminée
        if (userInputIndex >= currentSequenceSize) {
          Serial.println("Étape réussie !");

          // Progression dans la séquence ou niveau
          if (currentSequenceSize < maxSequenceSize) {
            currentSequenceSize++;
          } else 
          if (!levelTransition) { 
            levelTransition = true; 
            if (currentLevel == 1) {
              Serial.println("Niveau 1 terminé !");
              playSuccessMelody();
              showLevelCompleteLEDs(true); // LEDs vertes
              currentLevel = 2;
              currentSequenceSize = 1;
              maxSequenceSize = 8;
              updateProgressionLEDs();
            } else if (currentLevel == 2) {
              Serial.println("Niveau 2 terminé !");
              playSuccessMelody();
              showLevelCompleteLEDs(true); // LEDs vertes
              currentLevel = 3;
              currentSequenceSize = 1;
              maxSequenceSize = 10;
              updateProgressionLEDs();
            } else if (currentLevel == 3) {
                Serial.println("Niveau 3 terminé !");
                playSuccessMelody();
                showLevelCompleteLEDs(true);

                // Allume la 3ème LED pour indiquer la fin des 3 niveaux
                digitalWrite(PIN_LED3, HIGH);

                levelComplete = true;

                // Affiche en permanence le chiffre "1" sur le pad
                displayNumberOne();

                // Envoi MQTT pour signaler la fin
                client.publish(mqtt_topic, "finish");

                // Active un état d'attente pour écouter les messages MQTT
                levelComplete = true;
                Serial.println("En attente de réinitialisation...");
              }
          }
          resetGame();
        }
      } else {
        // Mauvaise entrée : réinitialise la séquence courante
        Serial.println("Mauvaise touche !");
        showLevelCompleteLEDs(false); // LEDs rouges pour échec
        playFailureMelody();
        resetGame();
      }
      break; // Sort de la boucle après avoir traité la touche
    }
  }

  // Réaffiche la séquence si aucune entrée n'est reçue dans le délai
  if (!inputReceived && (millis() - userInputLastTime >= inputTimeout)) {
    Serial.println("Aucune entrée détectée, réaffichage de la séquence !");
    userInputLastTime = millis(); // Réinitialise le timer
    currentStep = 0;              // Redémarre la séquence
    sequenceActive = true;        // Passe en mode affichage de la séquence
    userInputMode = false;        // Désactive le mode utilisateur
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

// Fonction pour réinitialiser le jeu Simon à l'état initial
void resetSystem() {
  // Réinitialise les variables globales
  currentSequenceSize = 1;
  maxSequenceSize = 5;
  currentStep = 0;
  userInputIndex = 0;
  sequenceActive = true;
  userInputMode = false;
  levelComplete = false;
  levelTransition = false;
  currentLevel = 1;

  // Éteint les LEDs de progression
  digitalWrite(PIN_LED1, LOW);
  digitalWrite(PIN_LED2, LOW);
  digitalWrite(PIN_LED3, LOW);

  // Éteint toutes les LEDs du pad
  blackout();

  if (client.publish("/capteur/simon/status", "waiting")) {
    Serial.println("Message 'waiting' envoyé avec succès.");
  } else {
    Serial.println("Échec de l'envoi du message 'waiting'.");
  }

  // Régénère une nouvelle séquence
  generateSequence();

  Serial.println("Système réinitialisé avec succès.");
}

void setup() {
  Serial.begin(115200);
  setupSPI();

  // Configurer la pin audio
  pinMode(AudioPin, OUTPUT);

  // Configurer Wi-Fi et MQTT
  setupWiFi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback); // Associe la fonction callback
  
  // Connexion au broker MQTT (et souscription)
  reconnectMQTT();

  // Initialisation des LEDs
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

  if (levelComplete) {
    // Maintient l'affichage du chiffre "1"
    displayNumberOne();
    delay(1000); // Réduit la charge CPU et stabilise l'affichage
    return;      // Retourne directement pour éviter d'exécuter le reste du code
  }

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
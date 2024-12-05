#include <TFT_eSPI.h> // Bibliothèque pour l'écran intégré TTGO

// Définition des pins des boutons
const int buttonPin1 = 33;
const int buttonPin2 = 32;
const int buttonPin3 = 27;
const int buttonPin4 = 15;

// Variables pour suivre l'état précédent des boutons
bool previousState1 = HIGH;
bool previousState2 = HIGH;
bool previousState3 = HIGH;
bool previousState4 = HIGH;

// Compteur des boutons appuyés
int pressedCount = 0;

// Variable pour garder en mémoire la valeur précédente du compteur
int lastPressedCount = -1;

// Initialisation de l'écran
TFT_eSPI tft = TFT_eSPI();

void setup() {
  Serial.begin(115200);
  Serial.println("Test Moniteur Série.");

  // Configuration des pins en entrée avec résistances pull-up internes
  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);
  pinMode(buttonPin3, INPUT_PULLUP);
  pinMode(buttonPin4, INPUT_PULLUP);

  // Initialisation de l'écran
  tft.init();
  tft.setRotation(0); // Mode portrait
  tft.fillScreen(TFT_BLACK); // Efface l'écran initialement
  tft.setTextSize(3);   // Taille du texte
  tft.setTextColor(TFT_WHITE); // Couleur de texte initiale

  // Positionnement du curseur au centre
  int16_t xPos = (tft.width() - 6 * 3 * 4) / 2; // Centrage horizontal pour "4/4"
  int16_t yPos = tft.height() / 2 - 30; // Centrage vertical

  tft.setCursor(xPos, yPos); // Position du texte
  tft.println("Ecran initialise");
  delay(2000); // Pause pour confirmer l'affichage

  Serial.println("Système prêt.");
}

void loop() {
  pressedCount = 0; // Réinitialisation du compteur

  // Lecture des états des boutons
  bool state1 = digitalRead(buttonPin1);
  bool state2 = digitalRead(buttonPin2);
  bool state3 = digitalRead(buttonPin3);
  bool state4 = digitalRead(buttonPin4);

    

  // Comptage des boutons appuyés
  pressedCount += (state1 == LOW) ? 1 : 0;
  pressedCount += (state2 == LOW) ? 1 : 0;
  pressedCount += (state3 == LOW) ? 1 : 0;
  pressedCount += (state4 == LOW) ? 1 : 0;

  // Mise à jour de l'écran uniquement si le compteur a changé
  if (pressedCount != lastPressedCount) {
    lastPressedCount = pressedCount;

    // Efface et met à jour l'écran
    tft.fillScreen(TFT_BLACK); // Efface l'écran pour la mise à jour
    int16_t xPos = (tft.width() - 6 * 3 * 4) / 2; // Centrage horizontal
    int16_t yPos = tft.height() / 2 - 30; // Centrage vertical
    tft.setCursor(xPos, yPos); // Position du texte

    // Couleur du texte selon le compteur
    if (pressedCount == 4) {
      tft.setTextColor(TFT_GREEN);  // Vert si 4/4
    } else {
      tft.setTextColor(TFT_RED);    // Rouge si moins que 4/4
    }

    tft.printf("%d/4", pressedCount); // Affiche le compteur
    Serial.printf("Compteur de boutons appuyés : %d/4\n", pressedCount);
  }

  delay(50); // Anti-rebond léger
}

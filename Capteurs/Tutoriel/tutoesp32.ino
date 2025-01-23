#include <WiFi.h>
#include <PubSubClient.h>

#define BROCHE_ENTREE 38 // Pin de lecture analogique

int valeurAnalogique;      // Valeur lue sur l'entrée analogique
float tensionAnalogique;   // Tension calculée à partir de la valeur lue
float luminositePourcentage; // Luminosité en pourcentage

// Plage de tensions observée
const float V_MIN = 2.0;
const float V_MAX = 3.16;

// Informations du réseau WiFi
const char* ssid = "RobotiqueCPE";     // Remplace par ton SSID
const char* motDePasse = "AppareilLunaire:DauphinRadio"; // Remplace par ton mot de passe WiFi

// Informations du serveur MQTT
const char* mqtt_server = "134.214.51.148";  // Remplace par l'adresse de ton broker MQTT
const int mqtt_port = 1883;                 // Port par défaut du serveur MQTT (1883)

WiFiClient espClient;
PubSubClient client(espClient);

// Callback pour recevoir les messages MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0'; // Terminer la chaîne de caractères
  String message = String((char*)payload);

  // Réagir à la commande de fin
  if (String(topic) == "/capteur/tutoriel/donnees") {
    if (message == "finish") {
      // Publier le statut "finish"
      client.publish("/capteur/tutoriel/status", "finish");
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Connexion au WiFi
  setup_wifi();

  // Connexion au serveur MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback); // Définir la fonction callback pour les messages MQTT

  // Configuration de la résolution pour le DAC de l'ESP32
  analogReadResolution(12); // Résolution en 12 bits (de 0 à 4095)

  // Souscrire au topic "donnees" pour écouter la commande de fin
  client.subscribe("/capteur/tutoriel/donnees");

  // Initialiser l'état comme "waiting"
  client.publish("/capteur/tutoriel/status", "waiting");
}

void loop() {
  // Vérifier la connexion au serveur MQTT
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Lecture de l'entrée analogique
  valeurAnalogique = analogRead(BROCHE_ENTREE);

  // Calcul de la tension lue
  tensionAnalogique = (3.3 * valeurAnalogique) / 4095.0;

  // Conversion de la tension en pourcentage de luminosité
  luminositePourcentage = ((tensionAnalogique - V_MIN) / (V_MAX - V_MIN)) * 100;

  // Condition pour limiter la luminosité entre 0% et 100%
  if (luminositePourcentage < 0) {
    luminositePourcentage = 0;
  } else if (luminositePourcentage > 100) {
    luminositePourcentage = 100;
  }

  // Publication de la luminosité sur le topic "/capteur/tutoriel/donnees"
  char payload[50];
  snprintf(payload, sizeof(payload), "Luminosité : %.2f %%", luminositePourcentage);
  client.publish("/capteur/tutoriel/donnees", payload);

  // Affichage des résultats
  Serial.print("Valeur de l'entrée analogique : ");
  Serial.print(valeurAnalogique);
  Serial.print(" (sur 4095)");
  Serial.print(" - Tension lue : ");
  Serial.print(tensionAnalogique);
  Serial.print(" V");
  Serial.print(" - Luminosité : ");
  Serial.print(luminositePourcentage);
  Serial.println(" %");

  delay(500); // Attendre 500 ms avant la prochaine lecture
}

void setup_wifi() {
  delay(10);
  // Connexion au réseau WiFi
  Serial.println();
  Serial.print("Connexion au réseau WiFi ");
  Serial.println(ssid);

  WiFi.begin(ssid, motDePasse);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connecté au WiFi");
  Serial.print("Adresse IP : ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Boucle jusqu'à ce que la connexion MQTT soit établie
  while (!client.connected()) {
    Serial.print("Tentative de connexion au serveur MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println("Connecté au serveur MQTT");
      client.subscribe("/capteur/tutoriel/donnees"); // Souscrire au topic "donnees" lors de la reconnexion
    } else {
      Serial.print("Échec, erreur : ");
      Serial.print(client.state());
      delay(5000);
    }
  }
}


# 🎮 Jeu Simon

Ce projet implémente un jeu de mémoire interactif basé sur le célèbre jeu **Simon**, en utilisant un pad LED, un système audio et une communication via MQTT pour suivre et réinitialiser l'état du jeu. Le joueur doit reproduire des séquences de couleurs et de sons générées aléatoirement pour progresser dans le jeu.

## 📜 Description du projet

Le jeu consiste à :
1. Générer des séquences aléatoires de LEDs (avec des couleurs et des sons associés).
2. Permettre au joueur de reproduire ces séquences en appuyant sur les boutons correspondants.
3. Passer au niveau suivant lorsque la séquence est correctement reproduite.
4. Afficher le chiffre "1" en LED et envoyer un message MQTT **finish** lorsque tous les niveaux sont complétés.

Le jeu inclut :
- Gestion des LEDs pour afficher des séquences et le progrès.
- Génération de sons correspondant aux LEDs.
- Gestion des niveaux, avec une difficulté croissante.
- Communication avec un serveur MQTT pour signaler l'état du jeu et écouter des commandes.

---

## 🔧 Fonctionnalités principales

1. **Jeu Simon classique** :
   - Affichage de séquences lumineuses sur un pad LED 4x4.
   - Génération de sons correspondant à chaque LED.

2. **Gestion des niveaux** :
   - Niveau 1 : Séquences de 5 étapes.
   - Niveau 2 : Séquences de 8 étapes.
   - Niveau 3 : Séquences de 10 étapes.
   - Affichage progressif des niveaux via des LEDs.

3. **État de fin de jeu** :
   - Une fois les 3 niveaux complétés :
     - Les LEDs affichent en permanence un chiffre "1".
     - Un message MQTT **finish** est envoyé.

4. **Communication MQTT** :
   - Envoi de l'état du jeu via MQTT :
     - **waiting** : Le jeu est prêt à démarrer.
     - **finish** : Tous les niveaux ont été complétés.
   - Réception de la commande MQTT **reset** pour réinitialiser le jeu.

---

## 🔧 Architecture technique

### 🔍 Structure globale du projet

```plaintext
.
├── Simon/
│   ├── Simon.ino         # Code principal du jeu Simon
│   └── README.md             # Documentation du projet

```

---

## 🖥️ Technologies utilisées

### Matériel
- **ESP32** : Microcontrôleur pour piloter le pad LED et la communication MQTT.
- **Pad LED 4x4** : Pour afficher les séquences et le chiffre "1".
- **Système audio** : Génération de sons associés aux LEDs.
- **LEDs de progression** : Indiquent les niveaux complétés.

### Logiciel
- **Arduino IDE** : Développement et déploiement du code.
- **Protocole MQTT** :
  - **PubSubClient** : Librairie utilisée pour la communication MQTT.
- **SPI** : Pour la communication avec le pad LED.

---

## 📡 MQTT : Communication entre le jeu et les autres systèmes

### Topics MQTT
- **Publication** :
  - `/capteur/simon/status` :
    - **waiting** : En attente de début ou après réinitialisation.
    - **finish** : Signal que tous les niveaux ont été complétés.
- **Souscription** :
  - `/capteur/simon/cmd` :
    - **reset** : Réinitialisation du jeu.


## 🚀 Déploiement et utilisation

### 1. Pré-requis
- **ESP32** :
  - Configuré pour le réseau Wi-Fi **RobotiqueCPE**.
- **Broker MQTT** :
  - Disponible à l'adresse `134.214.51.148` sur le port `1883`.
- **Arduino IDE** :
  - Installez les librairies nécessaires :
    - **PubSubClient**
    - **SPI**

### 2. Déploiement

1. **Téléversez le code** :
   - Configurez les paramètres Wi-Fi et MQTT dans le code.
   - Compilez et téléversez le fichier `.ino` sur l'ESP32.

2. **Exécutez le jeu** :
   - Alimentez l'ESP32 et attendez la connexion Wi-Fi.
   - Une séquence aléatoire s'affichera. Suivez-la pour progresser.

3. **Réinitialisation** :
   - Envoyez la commande MQTT **reset** pour recommencer le jeu.
# 🎮 Jeu ToF

Ce projet implémente un jeu interactif basé sur des capteurs **ToF** (Time of Flight), en utilisant des LEDs pour indiquer la proximité par rapport à une distance cible. Le but du jeu est de positionner des objets ou surfaces à des distances spécifiques des capteurs ToF. Lorsque les 4 capteurs atteignent leur distance cible, le jeu est considéré comme gagné.

## 📜 Description du projet

Le jeu consiste à :
1. Utiliser 4 capteurs **ToF** pour mesurer des distances.
2. Afficher des couleurs sur des LEDs en fonction de l'écart par rapport aux distances cibles.
3. Passer en état **finish** lorsque toutes les distances sont atteintes simultanément.
4. Attendre une commande MQTT **reset** pour réinitialiser le jeu.

Le jeu inclut :
- Gestion des LEDs pour représenter les distances mesurées.
- Une animation arc-en-ciel pour signaler le début ou la réinitialisation du jeu.
- Communication avec un serveur MQTT pour signaler l'état du jeu et écouter des commandes.

---

## 🔧 Fonctionnalités principales

1. **Mesure des distances avec des capteurs ToF** :
   - 4 capteurs VL53L0X configurés avec des adresses I²C spécifiques.
   - Mesure des distances en centimètres, avec une tolérance autour de la distance cible.

2. **Indication des distances avec LEDs** :
   - Rouge : Distance éloignée de la cible.
   - Vert : Distance dans la plage de tolérance.

3. **État de fin de jeu** :
   - Lorsque toutes les distances cibles sont atteintes :
     - Les LEDs s'affichent en vert permanent.
     - Un message MQTT **finish** est envoyé.

4. **Communication MQTT** :
   - Envoi de l'état du jeu via MQTT :
     - **waiting** : Le jeu est prêt à démarrer ou a été réinitialisé.
     - **finish** : Les distances cibles ont été atteintes.
   - Réception de la commande MQTT **reset** pour relancer le jeu.

---

## 🔧 Architecture technique

### 🔍 Structure globale du projet

```plaintext
.
├── ToF/
│   ├── ToF_Game.ino        # Code principal du jeu ToF
│   ├── README.md           # Documentation du projet
```

---

## 🖥️ Technologies utilisées

### Matériel
- **ESP32** : Microcontrôleur pour gérer les capteurs ToF, les LEDs et la communication MQTT.
- **VL53L0X** : Capteurs ToF pour mesurer les distances.
- **LEDs RGB** : Affichent les distances par des dégradés de couleur.
- **Ruban NeoPixel** : Gestion des LEDs via un ruban programmable.

### Logiciel
- **Arduino IDE** : Développement et téléversement du code.
- **Protocole MQTT** :
  - **PubSubClient** : Librairie utilisée pour la communication MQTT.
- **Wire (I²C)** : Communication entre l'ESP32 et les capteurs ToF.

---

## 📡 MQTT : Communication entre le jeu et les autres systèmes

### Topics MQTT
- **Publication** :
  - `/capteur/ToF/status` :
    - **waiting** : En attente de début ou après réinitialisation.
    - **finish** : Les distances cibles ont été atteintes.
- **Souscription** :
  - `/capteur/ToF/cmd` :
    - **reset** : Réinitialisation du jeu.

---

## 🚀 Déploiement et utilisation

### 1. Pré-requis

- **ESP32** :
  - Configuré pour le réseau Wi-Fi **RobotiqueCPE**.
- **Broker MQTT** :
  - Disponible à l'adresse `134.214.51.148` sur le port `1883`.
- **Arduino IDE** :
  - Installez les librairies nécessaires :
    - **PubSubClient**
    - **Adafruit_NeoPixel**
    - **Adafruit_VL53L0X**

### 2. Déploiement

1. **Téléversez le code** :
   - Configurez les paramètres Wi-Fi et MQTT dans le code.
   - Compilez et téléversez le fichier `.ino` sur l'ESP32.

2. **Exécutez le jeu** :
   - Alimentez l'ESP32 et attendez la connexion Wi-Fi.
   - Le ruban LED affiche une animation arc-en-ciel.
   - Les LEDs changent de couleur en fonction de la distance mesurée par chaque capteur.

3. **Réinitialisation** :
   - Envoyez la commande MQTT **reset** pour relancer le jeu.

4. **Gagner le jeu** :
   - Positionnez des objets ou surfaces à la distance cible pour tous les capteurs.
   - Les LEDs passent au vert, et un message **finish** est envoyé via MQTT.

---

## 📈 Développement

1. **Capteurs ToF** :
   - Chaque capteur a une adresse I²C unique et une distance cible définie.
   - Les distances mesurées sont affichées dans le moniteur série.

2. **Gestion des LEDs** :
   - Les LEDs reflètent les distances mesurées par un dégradé rouge-vert.

3. **Réinitialisation et victoire** :
   - Le système se réinitialise avec une commande MQTT **reset**.
   - Les LEDs passent au vert lorsque toutes les distances sont atteintes.

---

## 🛠️ Problèmes rencontrés et solutions

1. **Problèmes de détection des capteurs ToF** :
   - Solution : Initialisation séquentielle des capteurs avec des broches **XSHUT** distinctes.

2. **Synchronisation des distances cibles** :
   - Solution : Ajout d'une tolérance (`GREEN_TOLERANCE`) pour éviter des détections trop sensibles.

---

## ✨ Résultat attendu

- Les LEDs changent dynamiquement de couleur en fonction des distances mesurées par les capteurs ToF.
- Lorsqu'un joueur atteint les distances cibles sur les 4 capteurs, le jeu est gagné, les LEDs passent au vert, et un message **finish** est envoyé via MQTT.
- Une commande **reset** relance le jeu avec l'animation arc-en-ciel.
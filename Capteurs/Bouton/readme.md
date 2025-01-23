
# 🔧 Projet : Contrôle via 4 Boutons avec MQTT

Ce projet vise à créer un système de 4 boutons connectés, chacun capable de communiquer son état via le protocole MQTT. Une fois tous les boutons appuyés simultanément, un message "finish" est envoyé à Home Assistant. Ce dernier peut également envoyer une commande "reset" pour réinitialiser l'état du système.

--- 

## 📜 Description du projet

Le projet se compose de :

- **4 boutons physiques** connectés à des ESP32, chacun communiquant avec un broker MQTT pour indiquer son état ("pressed" ou "released").
- **Un script Python central** pour gérer les états des boutons, évaluer si tous les boutons sont appuyés, et envoyer un message "finish" lorsque les conditions sont remplies.
- **Home Assistant** pour envoyer la commande "reset" au système et recevoir les mises à jour d'état.

---

## 🛠️ Fonctionnalités principales

1. **Communication MQTT** :
   - Chaque bouton publie son état (à travers un ESP32) sur un topic unique.
   - Le script Python central souscrit à tous les topics pour surveiller les états des boutons.
   - Home Assistant peut envoyer une commande "reset" sur un topic dédié.

2. **Gestion des états des boutons** :
   - Les états des boutons sont suivis en temps réel par le script Python.
   - Lorsque tous les boutons sont appuyés simultanément, un message "finish" est publié.

3. **Réinitialisation du système** :
   - Une commande "reset" de Home Assistant réinitialise les états des boutons.

---

## 🛠️ Architecture technique

### 📂 Structure globale du projet

```plaintext
.
├── button1/
│   ├── button1.ino            # Code ESP32 pour le bouton 1
├── button2/
│   ├── button2.ino            # Code ESP32 pour le bouton 2
├── button3/
│   ├── button3.ino            # Code ESP32 pour le bouton 3
├── buttonLCD/
│   ├── buttonLCD.ino          # Code ESP32 pour le bouton 4 (LCD inclus)
├── python/
│   ├── main.py                # Script Python central pour la gestion des boutons
└── README.md                  # Documentation du projet

```

---

## 🖥️ Technologies utilisées


### Côté ESP32
- **Arduino IDE** : Pour programmer les ESP32.
- **WiFi** : Connexion des ESP32 au réseau.
- **PubSubClient** : Librairie pour gérer la communication MQTT.

### Côté serveur
- **Python 3** : Langage pour le script central.
- **paho-mqtt** : Librairie Python pour communiquer via MQTT.
- **Home Assistant** : Pour envoyer la commande "reset" et surveiller les états.

---

## 🚀 Déploiement et utilisation

### 1. Pré-requis

- **Broker MQTT** : Installez un broker MQTT tel que Mosquitto. Configurez-le pour être accessible par les ESP32 et le script Python.
- **ESP32** :
  - Connectez chaque ESP32 à votre réseau WiFi.
  - Chargez le code approprié (à partir des dossiers `button1/`, `button2/`, etc.) dans chaque ESP32.
- **Python** :
  - Installez Python 3 sur votre machine.
  - Installez la librairie `paho-mqtt` :
    ```bash
    pip install paho-mqtt
    ```

### 2. Configuration

#### ESP32

Dans chaque fichier `.ino`, configurez les informations suivantes :

- **SSID** et **mot de passe** de votre réseau WiFi :
  ```cpp
  const char* ssid = "RobotiqueCPE";
  const char* password = "AppareilLunaire:DauphinRadio";
  ```

- **Adresse du broker MQTT** :
  ```cpp
  const char* mqtt_server = "AdresseIPDuBroker";
  ```

#### Script Python

Dans `main.py`, configurez les informations suivantes :

- **Adresse du broker MQTT** :
  ```python
  BROKER = "AdresseIPDuBroker"
  PORT = 1883
  ```

### 3. Exécution

#### Côté ESP32

1. Connectez chaque ESP32 à votre ordinateur.
2. Chargez le code correspondant dans l'ESP32.
3. Redémarrez les ESP32 et assurez-vous qu'ils se connectent au WiFi et au broker MQTT.

#### Côté serveur
 Lancez le script Python :
   ```bash
   python3 button_monitor_mqtt.py
   ```


# 🔧 Tutoriel - Escape Game


## 📜 Description de l'atelier 
Ce projet fait partie d'un escape game autour de la robotique. Les participants interagissent avec un capteur de luminosité pour relever des défis progressifs. L'objectif est de guider les joueurs pour régler la luminosité selon des critères précis en recevant des retours en temps réel sur une interface graphique.

Une fois les défis complétés, un chiffre clé est révélé, permettant aux participants de progresser dans l'escape game.

## 🛠️ Fonctionnalités principales
- **Défis progressifs :**
  - Niveau 1 : Régler la luminosité entre 0 % et 5 %.
  - Niveau 2 : Régler la luminosité exactement à 70 %.
- **Retour en temps réel :** Affichage de la luminosité actuelle et des instructions sur une interface graphique.
- **Gestion des événements :** Réinitialisation et redémarrage possible via des commandes MQTT.
- **Retours sonores :** Des sons distinctifs signalent la réussite des défis.
- **Affichage visuel final :** Révélation d'un chiffre clé une fois les défis complétés.



## 🛠️ Architecture technique

### 📂 Structure globale du projet

```plaintext
├── tutoriel_lumineux.py # Script principal contenant l'interface et la logique MQTT 
|── tutoriel_esp32.ino # Code Arduino pour l'ESP32
├── defi_reussi.mp3 # Sons de validation des défis 
├── assets/ # Captures d'ecran du readme 
└── README.md # Documentation
```


## 🖥️ Technologies utilisées

- **Python :** Utilisé pour créer l'interface utilisateur et gérer la communication via MQTT.
  - Bibliothèques : `tkinter`, `paho-mqtt`, `pygame`.
- **LDR:** Permet de capter la luminosité
- **ESP32 :** Module matériel pour la lecture de la luminosité via une broche analogique.
  - Connectivité WiFi et protocole MQTT.
- **MQTT :** Protocole léger pour la communication entre l'ESP32 et le script Python. Envoi d'un reset permettant de recommencer l'atelier.
- **Arduino IDE :** Pour programmer l'ESP32.



## 🚀 Déploiement et utilisation

### 1. Pré-requis
- Python 3.8 ou plus.
- ESP32 configuré avec l'IDE Arduino.
- Broker MQTT actif (configuration incluse dans le code).
- Dépendances Python (`pip install paho-mqtt pygame`).

### 2. Déploiement
1. **Installation de l'interface utilisateur :**
   - Cloner le dépôt.
   - Installer les dépendances.
   - Lancer le script :  
     ```bash
     python tutoriel_lumineux.py
     ```

2. **Déploiement du firmware ESP32 :**
   - Configurer le code Arduino avec vos informations WiFi et MQTT.
   - Compiler et téléverser le code sur l'ESP32 via l'IDE Arduino.


## 🖥️ Captures d'écran

### Interface utilisateur 
*(Insérez une capture de l'écran principal avec le défi affiché)*

### Montage 
![Montage arriere](/assets/arriere.jpg)
![Montage avant](/assets/devant.jpg)

### Maquette 
![Maquette Tinkercad](/assets/maquette.png)

### Mise en route 
Voir README du lot d'atelier.


## 💡 Aide et Support
Pour toute question ou problème, vous pouvez ouvrir une issue sur le dépôt GitLab ou me contacter par email : elea.machillot@cpe.fr
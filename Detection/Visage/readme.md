# 📸 Reconnaissance de Visage avec Jeu Interactif

Ce projet implémente une application de reconnaissance faciale qui utilise une TTGO-CAM pour capturer des visages et vérifier s'ils correspondent à une personne pré-enregistrée. Une fois la personne correcte détectée pendant un laps de temps défini, le jeu affiche un chiffre "1" rose sur le flux vidéo. La communication avec un système externe (Home Assistant) est réalisée via MQTT pour les commandes et les statuts.

---

## 📜 Description du projet

L'application comprend :
- **Reconnaissance faciale** pour identifier une personne à partir d'un flux vidéo en direct.
- **Gestion de jeu** où un chronomètre se déclenche lorsqu'une personne spécifique est détectée.
- **Affichage interactif** d'un chiffre "1" rose sur le flux vidéo lorsque la condition est remplie.
- **Communication MQTT** pour transmettre l'état du jeu (états "waiting", "finish") et recevoir des commandes (état "reset").

---

## 🛠️ Fonctionnalités principales

1. **Reconnaissance faciale** :
   - Identification de visages à partir d'images pré-enregistrées.
   - Utilisation de la bibliothèque `face_recognition` pour l'encodage et la comparaison des visages.

2. **Jeu interactif** :
   - Chronomètre de 5 secondes activé lorsque la bonne personne est détectée.
   - Affichage d'un "1" rose lorsque la condition de victoire est atteinte.
   - Réinitialisation du jeu sur commande externe.

3. **Communication MQTT** :
   - Envoi du statut actuel (« waiting » ou « finish »).
   - Réception de commandes (« reset ») pour réinitialiser l'état du jeu.

---

## 🛠️ Architecture technique

### 📂 Structure globale du projet

```plaintext
Visage/
├── camera_visage/             # Gestion de la caméra CAM TTGO, il suffit de téléverser et la caméra se build
│   ├── app_httpd.cpp
│   ├── camera_index.h
│   ├── camera_pins.h
│   ├── camera_visage.ino
├── known_person/              # Dossier contenant les images de la personne connue
│   ├── photo_3.png
│   ├── photo_6.png
│   ├── photo_11.png
├── venv/                      # Environnement virtuel Python (à reproduire avec le requirements.txt)
├── cam_handler.py             # Script principal de gestion de la reconnaissance faciale
├── requirements.txt           # Dépendances Python
├── partitions.csv             # Configuration des partitions pour TTGO-CAM
└── readme.md                  # Documentation du projet
```

---

## 🖥️ Technologies utilisées

### Langages et bibliothèques :
- **Python** : Pour le traitement des images et la logique du jeu.
- **OpenCV** : Pour l'affichage en temps réel du flux vidéo et des overlays.
- **face_recognition** : Pour l'encodage et la comparaison des visages.
- **paho-mqtt** : Pour la communication MQTT avec Home Assistant.

### Matériel :
- **TTGO-CAM** : Caméra ESP32 pour capturer le flux vidéo.

### Système externe :
- **Home Assistant** : Pour centraliser les états et les commandes via MQTT.

---

## 🚀 Déploiement et utilisation

### 1. Pré-requis

- Python 3.9 ou supérieur.
- Les bibliothèques suivantes installées (via `requirements.txt`) :
  ```bash
  pip install -r requirements.txt
  ```
- Une TTGO-CAM configurée pour diffuser un flux vidéo.
- Un broker MQTT fonctionnel.

### 2. Lancer l'application

1. **Configurer les fichiers** :
   - Ajouter des images de la personne connue dans le dossier `known_person/`.
   - Mettre à jour l'adresse du flux vidéo et du broker MQTT dans `cam_handler.py` :
     ```python
     stream_url = "http://<ADRESSE_TTGO_CAM>:81/stream"
     BROKER = "<ADRESSE_BROKER_MQTT>"
     PORT = 1883
     ```

2. **Exécuter le script** :
   ```bash
   python3 cam_handler.py
   ```

3. **Interagir avec MQTT** :
   - Publier une commande `reset` sur le topic : `/detection/visage/cmd`
   - Observer les états publiés sur le topic : `/detection/visage/status`

---

## 📡 Communication MQTT

### Topics :
- **Commande** : `/detection/visage/cmd`
  - **reset** : Réinitialise le jeu.
- **Statut** : `/detection/visage/status`
  - **waiting** : En attente de la bonne personne.
  - **finish** : Jeu terminé (victoire).

### Exemple d'interaction avec Mosquitto :
- Envoyer une commande de réinitialisation :
  ```bash
  mosquitto_pub -h <BROKER> -t /detection/visage/cmd -m "reset"
  ```
- Souscrire aux messages de statut :
  ```bash
  mosquitto_sub -h <BROKER> -t /detection/visage/status
  ```

---

## 🔍 Fonctionnement du script principal

1. **Chargement des visages connus** :
   - Les images dans `known_person/` sont encodées pour permettre la reconnaissance.

2. **Flux vidéo** :
   - La TTGO-CAM diffuse un flux capté en temps réel par OpenCV.

3. **Détection faciale** :
   - `face_recognition` compare les visages du flux avec les encodages pré-enregistrés.

4. **Gestion du jeu** :
   - Le chronomètre commence lorsque la bonne personne est détectée.
   - Un chiffre "1" rose est affiché si la personne est détectée pendant 5 secondes.
   - Le statut "finish" est envoyé via MQTT.

5. **Commandes externes** :
   - Une commande `reset` via MQTT réinitialise le jeu et le flux vidéo.
s
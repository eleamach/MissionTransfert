# Jeu d'Énigmes Audio

Ce projet est un jeu d'énigmes interactif utilisant la reconnaissance vocale. Les joueurs doivent résoudre une série d'énigmes en parlant leurs réponses, qui sont ensuite évaluées par l'IA.

## Fonctionnalités

1. **5 Énigmes Initiales**
   - Série de 5 énigmes à résoudre dans l'ordre
   - Réponses vocales via le microphone
   - Évaluation en temps réel par GPT-4
   - Feedback immédiat sur la validité des réponses

2. **Interface Interactive**
   - Affichage de l'énigme courante
   - Transcription de la réponse donnée
   - Évaluation de l'IA
   - Bouton d'enregistrement style "radio"
   - Indicateur de progression

3. **Énigme Finale**
   - Débloquée après avoir résolu les 5 énigmes initiales
   - Énigme à deux parties
   - Interface simplifiée pour la phase finale

4. **Intégration MQTT**
   - Abonnement au topic `/ia/vocale/status`
   - Reset de l'application sur réception du message "reset"

## Installation

1. **Création de l'environnement virtuel**
python -m venv .venv
.venv\Scripts\activate

3. **Installation des dépendances**
pip install -r requirements.txt

4. **Démarrer l'application**
python app.py


## Configuration

1. Créer un fichier `.env` à la racine du projet :
```
OPENAI_API_KEY=votre_clé_api_openai
```

2. Ouvrir un navigateur et accéder à :
http://localhost:5000

## Jeu
1. Pour jouer :
   - Cliquer sur le bouton d'enregistrement pour commencer à parler
   - Cliquer à nouveau pour arrêter l'enregistrement
   - Attendre l'évaluation de l'IA
   - Continuer jusqu'à résoudre toutes les énigmes

2. Reset à distance :
   - Publier le message "reset" sur le topic MQTT `/ia/vocale/status`
   - L'application reviendra à son état initial

## Dépendances

Le fichier `requirements.txt` contient :
- flask
- openai
- python-dotenv
- sounddevice
- soundfile
- numpy
- flask-socketio
- paho-mqtt

## Notes

- Assurez-vous d'avoir un microphone fonctionnel
- L'application nécessite une connexion Internet pour :
  - L'API OpenAI (transcription et GPT-4)
  - Le broker MQTT (si distant)
- Le navigateur demandera la permission d'utiliser le microphone
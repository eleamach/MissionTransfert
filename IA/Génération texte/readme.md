# Jeu d'Énigmes Audio

Ce projet est un jeu d'énigmes interactif utilisant la generation de texte. Les joueurs doivent compter le nombre d'occurences d'un mot dans un texte dit oralement.

## Fonctionnalités

1. **Generation de texte**
   - Le texte est généré par un LLM
   - Le texte est prononcé orallement par un TTS
   - La requete est envoyé pendant la diction du texte, afin de proposer une reponse instantané lors de l'appui utilisateur


4. **Intégration MQTT**
   - Abonnement au topic `/ia/texte/status`
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

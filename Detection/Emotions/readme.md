# Epreuve emotion  

## Description 

## Installation

Dans un terminal : 
```bash
python -m venv venv_projet
source venv_projet/bin/activate

pip install -r requirements.txt

# Si erreur cv2.error tester
pip unistall opencv-python
pip install opencv-python>=4.5.5
```

### Prérequis
Vérifier la sortie vidéo dans le fichier [detection_emotion.py](detection_emotions.py) ligne 19

```py capture = cv2.VideoCapture(1, cv2.CAP_V4L2)```

```bash
# voir les sortie vidéo possible grace a la commande 
ls -l /dev/video*
```

### Étapes de lancement 
Dans le dossier [Emotions](.) lancer la commande : ```bash python detection_emotions.py``` 
Placer correctement les fenetres ouvertes

## Fonctionnalités

1. **Détection d’émotions avec plusieurs niveaux de difficulté**  
   Le jeu repose sur l’imitation d’émotions détectées par un modèle. Les niveaux sont conçus pour offrir une progression :  

   - **Niveau 1 : Reproduire les émotions dans un ordre prédéfini**  
     - Les joueurs doivent imiter une séquence d’émotions : **Triste, Heureux, Naturel**.  
     - L’ordre des émotions est indiqué par des images affichées dans la salle.  
     - Un timer de 40 secondes est imposé pour réussir ce niveau.  

   - **Niveau 2 : Reproduire des paires d’émotions**  
     - Les joueurs doivent imiter deux émotions affichées simultanément à l’écran, en fonction de la couleur associée.  
       - Émotions à reproduire :  
         - **Heureux et Triste.**  
         - **Surpris et Énervé.**  

   - **Niveau 3 : Reproduire une séquence basée sur une énigme**  
     - Une phrase énigmatique s’affiche à l’écran pour indiquer l’ordre des émotions à imiter.  
     - Émotions à reproduire : **Triste, Énervé, Dégoûté.**  
     - Un timer de 30 secondes est imposé pour réussir ce niveau.  

2. **Validation du succès**  
   - Lorsque le joueur réussit à imiter correctement toutes les émotions d’un niveau, un **code de validation** s’affiche sur l’écran.  

3. **Gestion via MQTT**  
   - **Réinitialisation du jeu** :  
     - Le jeu peut être réinitialisé en recevant la commande **"reset"** sur un topic MQTT dédié.  
   - **Publication d’état** :  
     - **"waiting"** : confirmation de la réinitialisation.  
     - **"finish"** : indication que l’atelier est terminé.  


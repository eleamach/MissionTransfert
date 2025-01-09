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

- Détection d'emotions en plisieur niveaux
    1. Imiter les émotions dans l'ordre Triste Heureux et Naturel
        - Ordre définie grace a des images dans la salle
    2. Imiter les émotions par paire grace au couleur a l'écran
        - Heureux Triste
        - Surpris Enervé
    3. Imiter les émotions dans l'ordre grace a une phrase énigmatique affiché a l'écran
        - Triste Enervé Dégoûté
- Subscription mqq a un topic pour reset le jeux
- Publication sur un topic pour confirmer le reset ou informer que l'atelier est fini
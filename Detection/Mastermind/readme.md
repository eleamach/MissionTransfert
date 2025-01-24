# Epreuve Mastermind  

## Description 

## Installation

Dans un terminal : 
```bash
# Creation initiale  du virtual env
python -m venv venv_projet
# Activation du virtual env, a refaire a chaque ouverture d'un terminal
source venv_projet/bin/activate
#installation des requirements du dossier Mastermind (à faire une fois avec le virtual env activé)
pip install -r requirements.txt
```

### Prérequis
Vérifier la sortie vidéo dans le fichier [mastermind.py](mastermind.py) ligne 19

```py capture = cv2.VideoCapture(1, cv2.CAP_V4L2)```

```bash
# voir les sortie vidéo possible grace a la commande 
ls -l /dev/video*
```

### Étapes d'installation
Dans le dossier [Mastermind](.) lancer la commande : 
```bash 
python mastermind.py
``` 


## Fonctionnalités

1. **Principes de base**  
   - Le jeu comprend 6 balles de couleurs différentes.  
   - Un support est placé devant une caméra pour permettre de positionner les balles.  
   - Une combinaison correcte de couleurs est générée aléatoirement par le programme.  

2. **Détection et comparaison des couleurs**  
   - Lorsqu’un joueur place 4 balles dans le support, le code Python détecte les couleurs dans les **ROI** (*Regions of Interest*) et les compare à la combinaison générée.  
   - Les couleurs sont évaluées selon deux critères :  
     - Bien placées : correspondance exacte de la couleur et de la position.  
     - Mal placées : la couleur est correcte mais mal positionnée.  

3. **Retour visuel et communication via MQTT**  
   - Les résultats de chaque tentative sont communiqués à un ESP32 via MQTT :  
     - Le nombre de balles **bien placées** est représenté par des LED vertes.  
     - Le nombre de balles **mal placées** est représenté par des LED rouges.  
   - Un compteur d'essais s'incrémente à chaque nouvelle tentative et s’affiche sur l’écran de l’ESP32.  

4. **Réinitialisation et génération de nouvelles combinaisons**  
   - Après 15 essais sans succès, une nouvelle combinaison est générée, et le compteur est réinitialisé.  
   - Si le joueur découvre la combinaison correcte (4 LED vertes allumées), un code de validation s’affiche sur l’écran.  

5. **Gestion via MQTT**  
   - Le jeu peut être réinitialisé en envoyant la commande **"reset"** à un topic MQTT dédié.  
   - Des messages sont publiés pour indiquer l’état du jeu :  
     - **"waiting"** : confirmation de la réinitialisation.  
     - **"finish"** : signalisation de la fin de l’atelier.  

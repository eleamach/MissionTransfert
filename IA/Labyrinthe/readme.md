# Epreuve génération d'image  

## Description 

## Installation

### Prérequis

### Étapes d'installation

## Fonctionnalités

1. **Principe du jeu**  
   Le jeu comporte 4 labyrinthes papier que le joueur doit résoudre. Chaque labyrinthe est associé à un ESP32 avec un écran et un digicode pour saisir les solutions.  

2. **Calcul du coût de déplacement**  
   - Le joueur doit calculer le **coût de déplacement** pour se rendre du point de départ au point d’arrivée dans chaque labyrinthe en utilisant l’**algorithme de Dijkstra**.  
   - Les labyrinthes augmentent en difficulté selon les critères suivants :  
     - **Premier labyrinthe** : petit labyrinthe sans poids, utilisé pour comprendre le fonctionnement du jeu.  
     - **Deuxième labyrinthe** : plus grand labyrinthe, mais toujours sans poids.  
     - **Troisième labyrinthe** : comme le premier, mais avec des **poids** ajoutés sur certaines cases, rendant certains déplacements plus coûteux.  
     - **Quatrième labyrinthe** : comme le deuxième, mais avec des **poids** sur certaines cases, augmentant la complexité.  
   - Une fois le chemin calculé, le joueur entre le **code correspondant** sur le digicode pour valider sa solution.  

3. **Validation et affichage du résultat**  
   - Lorsque les 4 labyrinthes sont résolus correctement et que le digicode a été entré pour chaque labyrinthe, un **chiffre de validation** s’affiche sur l’écran.  

4. **Gestion via MQTT**  
   - **Réinitialisation du jeu** :  
     - Le jeu peut être réinitialisé en recevant la commande **"reset"** sur un topic MQTT dédié.  
   - **Publication d’état** :  
     - **"waiting"** : confirmation de la réinitialisation.  
     - **"finish"** : indication que l’atelier est terminé.  


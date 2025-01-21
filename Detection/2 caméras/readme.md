## Objectif de l'attelier
2 cameras sont positionée à 90° l'une de l'autre.
Une doit voir 2 personnes, l'autre 4.
Pour resoudre l'enignme il faut donc ce "cacher" derriere une personne pour tromper l'une des cameras.

### ESP32-CAM #1
- IP : 10.51.11.59
- Port du flux vidéo : 81
- Souscrit au topic MQTT : `/detection/bi-camera/cam1-msg`
- Objectif : Détecter 4 personnes

### ESP32-CAM #2
- IP : 10.51.11.65
- Port du flux vidéo : 81
- Souscrit au topic MQTT : `/detection/bi-camera/cam2-msg`
- Objectif : Détecter 2 personnes

### Broker MQTT
- IP : 134.214.51.148
- Port : 1883
- Topics utilisés :
  - `/detection/bi-camera/cam1-msg` : Messages pour la caméra 1
  - `/detection/bi-camera/cam2-msg` : Messages pour la caméra 2
  - `/detection/bi-camera/cmd` : Commandes de contrôle (ex: "reset")

## Fonctionnalités

### Script Python (cameras_handler.py)
- Détection de personnes en temps réel avec YOLOv8
- Stabilisation des détections avec moyenne glissante
- Affichage du nombre de personnes détectées
- Communication MQTT pour informer les ESP32 du nombre de personnes
- Affichage "Bravo !! Rouge 1" lorsque l'objectif est atteint
- Reset via commande MQTT

### ESP32
 - Affiche le message envoyé sur le topic MQTT
 - Stream le flux video

## Notes
- Le seuil de confiance pour la détection est réglé à 0.5
- La stabilisation utilise une fenêtre glissante de 5 frames
- L'objectif est considéré comme atteint après 3 secondes de détection stable
# Lancement du jeu
```python
python game_master.py
```
# Topics MQTT

## Topics de commande (cmd)

### Lot "capteur"
- `capteur/bouton/cmd` : Commandes pour le workshop bouton
- `capteur/simon/cmd` : Commandes pour le workshop simon
- `capteur/tof/cmd` : Commandes pour le workshop tof

### Lot "detection"
- `detection/bi-camera/cmd` : Commandes pour le workshop bi-camera
- `detection/masterming/cmd` : Commandes pour le workshop masterming
- `detection/emotions/cmd` : Commandes pour le workshop emotions
- `detection/photo/cmd` : Commandes pour le workshop photo

### Lot "ia"
- `ia/texte/cmd` : Commandes pour le workshop texte
- `ia/vocale/cmd` : Commandes pour le workshop vocale
- `ia/labyrinthe/cmd` : Commandes pour le workshop labyrinthe

## Topics de status

### Lot "capteur"
- `capteur/bouton/status` : État du workshop bouton
- `capteur/simon/status` : État du workshop simon
- `capteur/tof/status` : État du workshop tof

### Lot "detection"
- `detection/bi-camera/status` : État du workshop bi-camera
- `detection/masterming/status` : État du workshop masterming
- `detection/emotions/status` : État du workshop emotions
- `detection/photo/status` : État du workshop photo

### Lot "ia"
- `ia/texte/status` : État du workshop texte
- `ia/vocale/status` : État du workshop vocale
- `ia/labyrinthe/status` : État du workshop labyrinthe

## Topics du jeu
- `/game/time` : Temps restant au format "MM:SS"

## Valeurs possibles

### Commandes (cmd)
- `reset` : Réinitialise le workshop

### Status
- `waiting` : Workshop en attente
- `finish` : Workshop terminé

## Phases du jeu
1. **PHASE_1** : Lot "capteur" fini
2. **PHASE_2** : Lot "detection" fini
3. **PHASE_3** : Lot "ia" fini

Le passage à la phase suivante se fait automatiquement quand tous les workshops de la phase courante sont terminés (`finish`).

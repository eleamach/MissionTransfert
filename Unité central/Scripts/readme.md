# Lancement du jeu
```python
python game_master.py
```
# Topics MQTT

## Topics de commande (cmd)

### Lot "capteur"
- `/capteur/bouton/cmd` : Commandes pour le workshop bouton
- `/capteur/simon/cmd` : Commandes pour le workshop simon
- `/capteur/tof/cmd` : Commandes pour le workshop tof

### Lot "detection"
- `/detection/bi-camera/cmd` : Commandes pour le workshop bi-camera
- `/detection/mastermind/cmd` : Commandes pour le workshop masterming
- `/detection/emotions/cmd` : Commandes pour le workshop emotions
- `/detection/photo/cmd` : Commandes pour le workshop photo

### Lot "ia"
- `/ia/texte/cmd` : Commandes pour le workshop texte
- `/ia/vocale/cmd` : Commandes pour le workshop vocale
- `/ia/labyrinthe/cmd` : Commandes pour le workshop labyrinthe

## Topics de status

### Lot "capteur"
- `/capteur/bouton/status` : État du workshop bouton
- `/capteur/simon/status` : État du workshop simon
- `/capteur/tof/status` : État du workshop tof

### Lot "detection"
- `/detection/bi-camera/status` : État du workshop bi-camera
- `/detection/masterming/status` : État du workshop masterming
- `/detection/emotions/status` : État du workshop emotions
- `/detection/photo/status` : État du workshop photo

### Lot "ia"
- `/ia/texte/status` : État du workshop texte
- `/ia/vocale/status` : État du workshop vocale
- `/ia/labyrinthe/status` : État du workshop labyrinthe

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



## Aide création pub-sub
récupérer le fichier [/Detection/Mastermind/mqtt_message.py](../../Detection/Mastermind/mqtt_message.py) 

Dans le python de base 
```py
from mqtt_message import start_mqtt_service #mqtt_message = nom du fichier
```

### Pour subscribe
Modifier le topic a la ligne 58,

Dans le python de base : 
```py 
if __name__ == '__main__':
    mqtt_service = start_mqtt_service(on_cmd_receive=lambda cmd: cmd_receive(cmd)) 
    # cmd_receive peut avoir d'autre parametre
```

Exemple de la fonction cmd_receive (avec un parametre en plus)
```py
def cmd_receive(cmd, game):
    if cmd == "reset":
        game.reset_game()
```

### Pour publish
Si pas de subscribe
```py 
if __name__ == '__main__':
    mqtt_service = start_mqtt_service() 
```

Pour envoyer : 
```py
# un json
data = {'correcte': game.correcte, 'mal_place': game.malPlace, 'essais' : f"{game.essais}/{game.essais_max}"}
mqtt_service.publish(data, "/detection/mastermind/led")

# un string             
data = "finish"
mqtt_service.publish(data, "/detection/masterming/status")
                    
```
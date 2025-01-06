# -*- coding: utf-8 -*-
import qi
from time import sleep
from mqtt_message import start_mqtt_service  # Import de la fonction start_mqtt_service

# Definitions globales
IP = "10.50.90.104"
PORT = 9559

def main():
    try:
        # Connexion au robot
        connection_url = "tcp://{}:{}".format(IP, PORT)
        app = qi.Application(["PepperRun", "--qi-url={}".format(connection_url)])
        app.start()
        session = app.session
        pepper_service = session.service("PepperService")
        
        # Initialisation du service MQTT avec la fonction callback pour les commandes reçues
        is_started = False  # Variable pour vérifier si le message 'start' a été reçu
        mqtt_service = start_mqtt_service(on_cmd_receive=lambda cmd: cmd_receive(cmd, pepper_service, is_started, mqtt_service))

        # Boucle d'attente jusqu'à recevoir le message "start"
        while not is_started:
            sleep(1)  # Attente de 1 seconde entre les vérifications

        # Après la réception du message "start", continuez avec la partie 1
        print("Démarrage de la partie 1")
        partie_1(pepper_service)
        
        # Publication d'un ensemble de message dès le dubut pour les initialiser
        mqtt_service.publish("waiting", "/pepper/status")
        mqtt_service.publish("false", "/pepper/capteur/valide")
        mqtt_service.publish("false", "/pepper/detection/valide")
        mqtt_service.publish("false", "/pepper/ia/valide")
        
        # Boucle d'attente pour recevoir les données des capteurs, de la détection et de l'IA
        print("Capteur data:", mqtt_service.capteur_data)
        print("Detection data:", mqtt_service.detection_data)
        print("IA data:", mqtt_service.ia_data)
        
        """ 
        pepper_service.raise_event("SimpleWeb/Page/Start", 1)
        partie_1(pepper_service)
        # DONE
        partie_2(pepper_service)
        sleep(5)
        partie_3(pepper_service)
        sleep(5)
        partie_4(pepper_service)
        sleep(15)
        partie_5(pepper_service)
        sleep(5)
        partie_6(pepper_service) 
        """
        
        # Arrêt du service MQTT après avoir envoyé les messages
        mqtt_service.stop()

    except RuntimeError as e:
        print("Error: {}".format(e))

def cmd_receive(cmd, pepper_service, is_started, mqtt_service):
    """ Fonction appelée à la réception d'une commande """
    if cmd == "start":
        print("Commande 'start' reçue, lancement de partie_1")
        mqtt_service.publish("In game", "/pepper/status")
        is_started = True 
        partie_1(pepper_service)
        return is_started 
    else:
        print("Commande non reconnue")
        return is_started


# Partie 1 : Introduction
def partie_1(pepper_service):
    pepper_service.play_music("/home/nao/.local/share/PackageManager/apps/elea/robot.mp3")
    """ pepper_service.speak_n_animate(0, "intro", "animations/Stand/BodyTalk/Listening/Listening_1", False, False)
    pepper_service.speak_n_animate(1, "intro", "animations/Stand/Gestures/Hey_6", False, False)
    pepper_service.speak_n_animate(2, "intro", "animations/Stand/BodyTalk/Listening/Listening_1", False, False)
    pepper_service.speak_n_animate(3, "intro", "animations/Stand/Reactions/TouchHead_4", False, False)
    pepper_service.speak_n_animate(6, "intro", "animations/Stand/Emotions/Positive/Laugh_2", False, False)
    pepper_service.speak_n_animate(4, "intro", "animations/Stand/BodyTalk/Listening/Listening_1", False, False)
    pepper_service.speak_n_animate(5, "intro", "animations/Stand/BodyTalk/Listening/Listening_1", False, False)
    pepper_service.raise_event("SimpleWeb/Page/Progress0", 1) """
    pepper_service.play_music("/home/nao/.local/share/PackageManager/apps/elea/clock.mp3")

# Partie 2 : Premier lot "Capteurs"
def partie_2(pepper_service):
    pepper_service.raise_event("SimpleWeb/Page/Start", 1)
    pepper_service.speak_n_animate(0, "premierlot", "animations/Stand/Waiting/Robot_1", False, False)
    pepper_service.speak_n_animate(1, "premierlot", "animations/Stand/Emotions/Positive/Laugh_2", False, False)
    pepper_service.speak_n_animate(2, "premierlot", "animations/Stand/Emotions/Negative/Fear_1", True, False)
    pepper_service.speak_n_animate(3, "premierlot", "animations/Stand/Emotions/Negative/Fear_1", False, False)
    pepper_service.raise_event("SimpleWeb/Page/Progress33", 1)
    pepper_service.play_music("/home/nao/.local/share/PackageManager/apps/elea/clock.mp3")

# Partie 3 : Deuxième lot "Detection"
def partie_3(pepper_service):
    pepper_service.raise_event("SimpleWeb/Page/Start", 1)
    pepper_service.speak_n_animate(0, "deuxiemelot", "animations/Stand/Waiting/Robot_1", False, False)
    pepper_service.speak_n_animate(1, "deuxiemelot", "animations/Stand/Emotions/Negative/Fear_1", True, False)
    pepper_service.speak_n_animate(2, "deuxiemelot", "animations/Stand/Emotions/Negative/Fear_1", False, False)
    pepper_service.raise_event("SimpleWeb/Page/Progress66", 1)
    pepper_service.play_music("/home/nao/.local/share/PackageManager/apps/elea/clock.mp3")

# Partie 4 : Troisième lot "IA"
def partie_4(pepper_service):
    pepper_service.raise_event("SimpleWeb/Page/Start", 1)
    pepper_service.speak_n_animate(0, "deuxiemelot", "animations/Stand/Waiting/Robot_1", False, False)
    pepper_service.speak_n_animate(0, "code", "animations/Stand/BodyTalk/Listening/Listening_1", False, False)
    pepper_service.speak_n_animate(1, "code", "animations/Stand/Emotions/Positive/Confident_1", False, False)
    pepper_service.speak_n_animate(2, "code", "animations/Stand/Gestures/YouKnowWhat_1", False, False)
    pepper_service.raise_event("SimpleWeb/Page/FinalButton", 1)

# Partie 5 : Vous avez réussi
def partie_5(pepper_service):
    pepper_service.raise_event("SimpleWeb/Page/Reussi", 1)
    pepper_service.speak_n_animate(0, "finalreussi", "animations/Stand/Waiting/Robot_1", False, False)
    pepper_service.speak_n_animate(1, "finalreussi", "animations/Stand/Emotions/Negative/Fear_1", True, False)
    pepper_service.speak_n_animate(2, "finalreussi", "animations/Stand/Emotions/Negative/Fear_1", False, False)
    pepper_service.speak_n_animate(3, "finalreussi", "animations/Stand/Emotions/Negative/Fear_1", True, False)
    pepper_service.speak_n_animate(4, "finalreussi", "animations/Stand/Emotions/Negative/Fear_1", False, False)
    pepper_service.speak_n_animate(5, "finalreussi", "animations/Stand/Gestures/Kisses_1", False, False)

# Partie 6 : Vous avez perdu
def partie_6(pepper_service):
    pepper_service.raise_event("SimpleWeb/Page/Start", 1)
    pepper_service.speak_n_animate(0, "finalperdu", "animations/Stand/BodyTalk/Listening/Listening_1", False, False)
    pepper_service.speak_n_animate(1, "finalperdu", "animations/Stand/BodyTalk/Listening/Listening_1", False, False)
    pepper_service.speak_n_animate(2, "finalperdu", "animations/Stand/BodyTalk/Listening/Listening_1", False, False)
    pepper_service.raise_event("SimpleWeb/Page/Perdu", 1)
    pepper_service.speak_n_animate(3, "finalperdu", "animations/Stand/BodyTalk/Listening/Listening_1", False, False)
    pepper_service.speak_n_animate(4, "finalperdu", "animations/Stand/BodyTalk/Listening/Listening_1", False, True)

if __name__ == "__main__":
    main()

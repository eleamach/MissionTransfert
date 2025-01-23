import paho.mqtt.client as mqtt
import tkinter as tk
from tkinter import ttk
import pygame  # Pour jouer des sons
import time

# Informations du serveur MQTT
MQTT_SERVER = "134.214.51.148"  # Adresse du broker MQTT
MQTT_PORT = 1883               # Port par défaut du serveur MQTT
MQTT_TOPIC = "/capteur/tutoriel/donnees"  # Topic à écouter
MQTT_CMD_TOPIC = "/capteur/tutoriel/cmd"  # Topic pour redémarrer le tutoriel

# Variables de contrôle
mqtt_client = None
current_defi = 1  # Défi actuel (1 ou 2)

# Limites des défis
DEFI_1_RANGE = (0, 5)   # Luminosité entre 0% et 5% pour le premier défi
DEFI_2_TARGET = 70      # Luminosité de 70% pour le deuxième défi (c'est un nombre simple)

# Instructions des défis
DEFI_INSTRUCTIONS = {
    1: "Niveau 1 : Réglez la luminosité entre 0% et 5%.",
    2: "Niveau 2 : Réglez la luminosité à exactement 70%.",
}

# Fonction pour jouer un son
def jouer_son(fichier):
    pygame.mixer.init()
    pygame.mixer.music.load(fichier)
    pygame.mixer.music.play()

# Fonction pour gérer le démarrage du tutoriel
def commencer_tutoriel():
    global current_defi  # Déclarer 'current_defi' comme global ici
    current_defi = 1  # Commencer toujours au défi 1
    print("Tutoriel démarré, en attente de messages MQTT...")

    # Faire disparaître le bouton de démarrage
    bouton_start.pack_forget()

    # Afficher les instructions du premier défi
    afficher_instruction_defi(current_defi)

    # Démarrer la boucle MQTT
    mqtt_client.loop_start()

# Fonction pour afficher les instructions du défi actuel
def afficher_instruction_defi(defi):
    if defi in DEFI_INSTRUCTIONS:
        instruction_label.config(text=DEFI_INSTRUCTIONS[defi])
    else:
        instruction_label.config(text="")

# Callback pour la connexion au broker MQTT
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connecté au broker MQTT")
        client.subscribe(MQTT_TOPIC)  # Souscrire au topic des données de luminosité
        client.subscribe(MQTT_CMD_TOPIC)  # Souscrire au topic de commande de redémarrage
    else:
        print(f"Échec de la connexion, code de retour : {rc}")

# Callback pour la réception de messages MQTT
def on_message(client, userdata, msg):
    global current_defi  # Déclarer 'current_defi' comme global ici pour le modifier
    message = msg.payload.decode("utf-8")  # Décoder le message reçu
    print(f"Message reçu sur le topic {msg.topic} : {message}")

    # Si le message est reçu sur le topic de redémarrage, réinitialiser l'interface
    if msg.topic == MQTT_CMD_TOPIC:
        if message.lower() == "restart":
            restart_tutoriel()
        return

    # Extraire la valeur de la luminosité (supprimer "Luminosité : " et "%")
    try:
        luminosite_str = message.split(":")[1].strip()  # Extraire la partie après "Luminosité :"
        luminosite_str = luminosite_str.replace(" %", "")  # Supprimer le symbole '%'
        luminosite = float(luminosite_str)  # Convertir en float
    except ValueError:
        print("Erreur : La luminosité reçue n'est pas un nombre valide.")
        return
    except IndexError:
        print("Erreur : Le message reçu n'est pas au format attendu.")
        return

    # Afficher la luminosité dans l'interface graphique
    luminosite_label.config(text=f"Luminosité actuelle : {luminosite}%")

    # Vérifier si le défi actuel est réussi
    if current_defi == 1:
        if DEFI_1_RANGE[0] <= luminosite <= DEFI_1_RANGE[1]:
            label_message.config(text="Bravo ! Vous avez réussi le premier défi.")
            jouer_son("defi_reussi.mp3")  # Son pour le défi 1
            current_defi = 2
            afficher_instruction_defi(current_defi)  # Mettre à jour les instructions
    elif current_defi == 2:
        if 70 <= luminosite <= 71:
            label_message.config(text="Félicitation, vous avez réussi le tutoriel. Voici votre premier chiffre : ")
            instruction_label.config(text="")  # Effacer les instructions
            luminosite_label.config(text="")  # Effacer la luminosité affichée

            # Ajouter un label pour afficher le chiffre 5 en bleu
            chiffre_label.config(text="5", fg="blue", font=("Arial", 128))  # Texte plus grand

            jouer_son("defi_reussi.mp3")  # Son pour le défi 2
            # Ne pas arrêter la boucle MQTT ici
            # Juste publier le message "finish"
            mqtt_client.publish(MQTT_TOPIC, "finish")  # Publier le message "finish"


# Fonction pour redémarrer le tutoriel et la boucle MQTT
def restart_tutoriel():
    global current_defi  # Réinitialiser les variables
    current_defi = 1  # Revenir au défi initial

    # Réinitialiser l'interface
    bouton_start.pack(pady=10)

    # Réinitialiser les autres éléments de l'interface
    label_message.config(text="Tutoriel de l'escape game")
    instruction_label.config(text="")
    luminosite_label.config(text="")
    chiffre_label.config(text="")  # Enlever le chiffre "5" quand le tutoriel redémarre

    # Redémarrer proprement la connexion MQTT et la boucle
    try:
        print("Redémarrage du tutoriel et réinitialisation de MQTT...")
        
        # Déconnecter proprement du client MQTT
        mqtt_client.disconnect()
        time.sleep(1)  # Attendre pour garantir la déconnexion

        # Reconnexion complète
        mqtt_client.reconnect()  # Cette méthode rétablit la connexion sans nécessiter de nouvelle connexion manuelle
        
        # Réabonnement aux topics
        mqtt_client.subscribe(MQTT_TOPIC)
        mqtt_client.subscribe(MQTT_CMD_TOPIC)
        
        # Démarrer la boucle MQTT sans la stopper
        mqtt_client.loop_start()  # Démarrer à nouveau la boucle MQTT
        print("MQTT réinitialisé et redémarré.")
    except Exception as e:
        print(f"Erreur lors du redémarrage de MQTT : {e}")

# Initialisation de la fenêtre principale avec tkinter
root = tk.Tk()
root.title("Tutoriel de l'escape game")
root.geometry("1920x1080")  # Dimension initiale de la fenêtre (plein écran)
root.attributes('-fullscreen', True)  # Passer en plein écran
root.resizable(False, False)

# Ajouter un message principal
label_message = tk.Label(root, text="Tutoriel de l'escape game", font=("Arial", 24), wraplength=400)
label_message.pack(pady=20)

# Ajouter un bouton pour commencer le tutoriel
bouton_start = ttk.Button(root, text="Je commence le tutoriel", command=commencer_tutoriel)
bouton_start.pack(pady=10)

# Ajouter un label pour afficher les instructions du défi actuel
instruction_label = tk.Label(root, text="", font=("Arial", 24), fg="green", wraplength=400)
instruction_label.pack(pady=20)

# Ajouter un label pour afficher la luminosité en temps réel
luminosite_label = tk.Label(root, text="", font=("Arial", 24), fg="blue")
luminosite_label.pack(pady=20)

# Ajouter un label pour afficher le chiffre 5 en bleu (initialisé ici)
chiffre_label = tk.Label(root, font=("Arial", 128), fg="blue")  # Texte plus grand et centré
chiffre_label.pack(pady=10)

# Initialisation du client MQTT
mqtt_client = mqtt.Client("PythonClient")
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message

try:
    # Connexion au broker MQTT
    mqtt_client.connect(MQTT_SERVER, MQTT_PORT, 60)

    # Démarrer l'interface graphique
    print("Lancement de l'interface graphique...")
    root.mainloop()
except KeyboardInterrupt:
    print("Déconnexion...")
    mqtt_client.disconnect()
except Exception as e:
    print(f"Erreur : {e}")

import paho.mqtt.client as mqtt

# Broker MQTT
BROKER = "134.214.51.148"
PORT = 1883

# Topics
BUTTON_TOPICS = ["/capteur/bouton/etat1", "/capteur/bouton/etat2", "/capteur/bouton/etat3", "/capteur/bouton/etat4LCD"]
STATUS_TOPIC = "/capteur/bouton/status"
CMD_TOPIC = "/capteur/bouton/cmd"

# Suivi des états des boutons
button_states = {topic: "released" for topic in BUTTON_TOPICS}

# Fonction pour afficher les états des boutons (debug)
def debug_button_states():
    print("État des boutons :")
    for topic, state in button_states.items():
        print(f"  {topic}: {state}")
    print("-" * 40)

# Gestion de la commande reset
def handle_reset_command(client):
    print("[INFO] Commande 'reset' reçue. Mise à jour de l'état de l'atelier.")
    # Envoie du message "waiting" pour signaler que l'atelier est en attente
    client.publish(STATUS_TOPIC, "waiting")
    print("[INFO] Message 'waiting' envoyé.")

# Callback pour la réception des messages
def on_message(client, userdata, msg):
    global button_states
    topic = msg.topic
    payload = msg.payload.decode()

    if topic in BUTTON_TOPICS:
        # Mise à jour de l'état du bouton
        previous_state = button_states[topic]
        button_states[topic] = payload

        # Affichage de debug si l'état change
        if payload != previous_state:
            print(f"[DEBUG] Bouton '{topic}' est maintenant : {payload}")
            debug_button_states()

        # Vérifie si tous les boutons sont pressés
        if all(state == "pressed" for state in button_states.values()):
            print("[INFO] Tous les boutons sont pressés. Envoi de 'finish'.")
            client.publish(STATUS_TOPIC, "finish")

    elif topic == CMD_TOPIC:
        # Vérifie si le message est "reset"
        if payload.lower() == "reset":
            handle_reset_command(client)

# Configuration MQTT
client = mqtt.Client()
client.on_message = on_message

# Connexion au broker
client.connect(BROKER, PORT)

# Souscription à tous les topics
for topic in BUTTON_TOPICS:
    client.subscribe(topic)

# Souscription au topic de commande
client.subscribe(CMD_TOPIC)

print("[INFO] En attente des messages...")
debug_button_states()
client.loop_forever()
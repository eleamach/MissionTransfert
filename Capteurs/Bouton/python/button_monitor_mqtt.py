import paho.mqtt.client as mqtt

# Broker MQTT
BROKER = "134.214.51.148"
PORT = 1883

# Topics
TOPICS = ["/capteur/bouton/etat1", "/capteur/bouton/etat2", "/capteur/bouton/etat3", "/capteur/bouton/etat4LCD"]
STATUS_TOPIC = "/capteur/bouton/status"

# Suivi des états des boutons
button_states = {topic: "released" for topic in TOPICS}

# Fonction pour afficher les états des boutons (debug)
def debug_button_states():
    print("État des boutons :")
    for topic, state in button_states.items():
        print(f"  {topic}: {state}")
    print("-" * 40)

# Callback pour la réception des messages
def on_message(client, userdata, msg):
    global button_states
    topic = msg.topic
    payload = msg.payload.decode()

    # Mise à jour de l'état du bouton
    if topic in button_states:
        previous_state = button_states[topic]
        button_states[topic] = payload

        # Affichage de debug
        if payload != previous_state:
            print(f"[DEBUG] Bouton '{topic}' est maintenant : {payload}")
            debug_button_states()

        # Vérifie si tous les boutons sont pressés
        if all(state == "pressed" for state in button_states.values()):
            print("[INFO] Tous les boutons sont pressés. Envoi de 'finish'.")
            client.publish(STATUS_TOPIC, "finish")

# Configuration MQTT
client = mqtt.Client()
client.on_message = on_message

# Connexion au broker
client.connect(BROKER, PORT)

# Souscription à tous les topics
for topic in TOPICS:
    client.subscribe(topic)

print("[INFO] En attente des messages...")
debug_button_states()
client.loop_forever()
import paho.mqtt.client as mqtt

# Informations sur le broker MQTT
BROKER = "134.214.51.148"
BUTTON_TOPICS = ["button1", "button2", "button3", "button4"]
OUTPUT_TOPIC = "buttonCount"

# Variables pour suivre les états des boutons
button_states = {topic: False for topic in BUTTON_TOPICS}

def on_connect(client, userdata, flags, rc):
    print(f"Connecté au broker MQTT avec le code de retour {rc}")
    for topic in BUTTON_TOPICS:
        client.subscribe(topic)
    print(f"Abonné aux topics : {BUTTON_TOPICS}")

def on_message(client, userdata, msg):
    global button_states
    topic = msg.topic
    payload = msg.payload.decode()

    # Mise à jour de l'état du bouton
    if topic in button_states:
        button_states[topic] = (payload == "appuye")
        print(f"{topic} : {'Appuyé' if button_states[topic] else 'Relâché'}")

        # Calculer le compteur des boutons appuyés
        total_pressed = sum(button_states.values())
        print(f"Boutons appuyés : {total_pressed}/4")

        # Publier le compteur sur le topic dédié
        message = {
            "count": f"{total_pressed}/4",
            "color": "green" if total_pressed == 4 else "red"
        }
        client.publish(OUTPUT_TOPIC, f"{message['count']},{message['color']}")
        print(f"Publié sur {OUTPUT_TOPIC} : {message}")

def main():
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    client.connect(BROKER, 1883, 60)

    # Boucle principale
    client.loop_forever()

if __name__ == "__main__":
    main()
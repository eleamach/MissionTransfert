# -*- coding: utf-8 -*-
import json
import time
import paho.mqtt.client as mqtt_client

class MqttService:
    def __init__(self, broker, port, client_id, on_cmd_receive=None):
        self.broker = broker
        self.port = port
        self.client_id = client_id
        self.client = None
        self.running = False
        self.on_cmd_receive = on_cmd_receive
        self.connected = False
        self.capteur_data = None  # Ajouter un attribut pour stocker les données du capteur
        self.detection_data = None  # Ajouter un attribut pour stocker les données de détection
        self.ia_data = None  # Ajouter un attribut pour stocker les données de l'IA
        
    def on_connect(self, client, userdata, flags, rc):
        """ Callback pour la connexion réussie au broker """
        if rc == 0:
            print("Connected to MQTT Broker!")
            self.connected = True
            # Souscription après connexion
            self.client.subscribe("/pepper/cmd")
            self.client.subscribe("/pepper/capteur/status")
            self.client.subscribe("/pepper/detection/status")
            self.client.subscribe("/pepper/ia/status")
        else:
            print("Failed to connect, return code {}".format(rc))

    def publish(self, msg, topic):
        """ Publie un message sur un topic MQTT """
        if not self.connected:
            print("Waiting for connection to the broker...")
            for _ in range(5):  # Attendre jusqu'à 5 secondes
                if self.connected:
                    break
                time.sleep(1)
        if self.client and self.connected:
            if isinstance(msg, dict):
                msg = json.dumps(msg)
            result = self.client.publish(topic, msg)
            status = result.rc
            if status == mqtt_client.MQTT_ERR_SUCCESS:
                print("Sent `{}` to topic `{}`".format(msg, topic))
            else:
                print("Failed to send message to topic `{}`".format(topic))
        else:
            print("Unable to send message: not connected to broker.")

    def connect(self):
        """ Crée et connecte le client MQTT """
        self.client = mqtt_client.Client(self.client_id)
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message  # Définir le callback pour la réception des messages
        self.client.connect(self.broker, self.port)

    def run(self):
        """ Démarre le service MQTT """
        self.running = True
        self.connect()
        self.client.loop_start()  # Démarre la boucle dans un thread séparé pour recevoir les messages

    def stop(self):
        """ Arrête le service MQTT """
        if self.client:
            self.client.loop_stop()
            print("MQTT Service stopped")

    def on_message(self, client, userdata, message):
        """ Callback pour traiter les messages reçus """
        print("Message reçu sur le topic {}: {}".format(message.topic, message.payload.decode()))

        # Vérifier quel topic a envoyé le message et envoyer les informations vers d'autres fonctions ou modules
        if message.topic == "/pepper/cmd":
            self.on_cmd_receive(message.payload.decode())
        elif message.topic == "/pepper/capteur/status":
            self.capteur_data = message.payload.decode()
        elif message.topic == "/pepper/detection/status":
            self.detection_data = message.payload.decode()
        elif message.topic == "/pepper/ia/status":
            self.ia_data = message.payload.decode()
        else:
            print("Topic non géré : {}".format(message.topic))

                
def start_mqtt_service(on_cmd_receive):
    """ Démarre le service MQTT et gère les messages reçus """
    # Initialiser le service avec une fonction callback pour la réception des commandes
    mqtt_service = MqttService(
        broker="134.214.51.148",  # Adresse du broker
        port=1883,                # Port
        client_id="python-mqtt-client",  # ID du client
        on_cmd_receive=on_cmd_receive  # Fonction callback pour traiter les commandes reçues
    )
    mqtt_service.run()  # Démarre le service MQTT
    
    return mqtt_service

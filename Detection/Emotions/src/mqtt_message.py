import json
import time 
import threading 
import paho.mqtt.client as mqtt_client

class MqttService:
    def __init__(self, broker, port, client_id, on_cmd_receive=None):
        self.broker = broker
        self.port = port
        self.client_id = client_id
        self.client = None
        self.running = False
        self.on_cmd_receive = on_cmd_receive 

    def on_connect(self, client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker!")
            self.client.subscribe("/+/+/cmd")  # Souscription au topic cmd
        else:
            print(f"Failed to connect, return code {rc}")

    def connect(self):
        # Crée un client MQTT
        self.client = mqtt_client.Client(self.client_id)
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message  # Assurez-vous que cette fonction est bien définie pour la gestion des messages
        self.client.connect(self.broker, self.port)

    def run(self):
        """ Démarre le service MQTT dans un thread séparé """
        self.running = True
        self.connect()
        self.client.loop_start()  # Démarre la boucle MQTT dans un thread séparé

    def stop(self):
        """ Arrête le service MQTT """
        if self.client:
            self.client.loop_stop()
            print("MQTT Service stopped")

    def publish(self, msg, topic):
        """ Publie un message sur un topic MQTT """
        if self.client and self.running:
            # Convertir le message en chaîne JSON
            if isinstance(msg, dict):
                msg = json.dumps(msg)
            result = self.client.publish(topic, msg)
            status = result.rc
            if status == mqtt_client.MQTT_ERR_SUCCESS:
                print(f"Sent `{msg}` to topic `{topic}`")
            else:
                print(f"Failed to send message to topic `{topic}`")

    def on_message(self, client, userdata, message):
        """ Callback pour traiter les messages reçus """
        print(f"Message reçu sur le topic {message.topic}: {message.payload.decode()}")

        if message.topic == "/detection/emotions/cmd" and self.on_cmd_receive:
            self.on_cmd_receive(message.payload.decode())


# Fonction qui crée une instance de service MQTT et l'exécute dans un thread séparé
def start_mqtt_service(on_cmd_receive=None):
    broker = '134.214.51.148'
    port = 1883
    client_id = 'python-mqtt-1'

    mqtt_service = MqttService(broker, port, client_id)
    mqtt_service.on_cmd_receive = on_cmd_receive
    
    # Démarre le service MQTT dans un thread séparé
    mqtt_thread = threading.Thread(target=mqtt_service.run)
    mqtt_thread.daemon = True  # Permet au thread de s'arrêter lorsque le programme principal termine
    mqtt_thread.start()

    return mqtt_service

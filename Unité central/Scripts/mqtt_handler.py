import paho.mqtt.client as mqtt
from typing import Callable

class MQTTHandler:
    # Command topics for "capteur" lot
    TOPIC_BUTTON_CMD = "/capteur/bouton/cmd"
    TOPIC_SIMON_CMD = "/capteur/simon/cmd"
    TOPIC_TOF_CMD = "/capteur/tof/cmd"
    
    # Command topics for "detection" lot
    TOPIC_BICAMERA_CMD = "/detection/bi-camera/cmd"
    TOPIC_MASTERMIND_CMD = "/detection/mastermind/cmd"
    TOPIC_EMOTIONS_CMD = "/detection/emotions/cmd"
    TOPIC_PHOTO_CMD = "/detection/photo/cmd"
    
    # Command topics for "ia" lot
    TOPIC_TEXTE_CMD = "/ia/texte/cmd"
    TOPIC_VOCALE_CMD = "/ia/vocale/cmd"
    TOPIC_LABYRINTHE_CMD = "/ia/labyrinthe/cmd"

    # Command topic pepper
    TOPIC_PEPPER_CMD = "/pepper/cmd"
    
    # Status topics for "capteur" lot
    TOPIC_BUTTON_STATUS = "/capteur/bouton/status"
    TOPIC_SIMON_STATUS = "/capteur/simon/status"
    TOPIC_TOF_STATUS = "/capteur/tof/status"
    
    # Status topics for "detection" lot
    TOPIC_BICAMERA_STATUS = "/detection/bi-camera/status"
    TOPIC_MASTERMIND_STATUS = "/detection/mastermind/status"
    TOPIC_EMOTIONS_STATUS = "/detection/emotions/status"
    TOPIC_PHOTO_STATUS = "/detection/photo/status"
    
    # Status topics for "ia" lot
    TOPIC_TEXTE_STATUS = "/ia/texte/status"
    TOPIC_VOCALE_STATUS = "/ia/vocale/status"
    TOPIC_LABYRINTHE_STATUS = "/ia/labyrinthe/status"

    # Status topic pepper
    TOPIC_PEPPER_STATUS = "/pepper/status"
    TOPIC_PEPPER_CAPTEUR_STATUS = "/pepper/capteur/status"
    TOPIC_PEPPER_DETECTION_STATUS = "/pepper/detection/status"
    TOPIC_PEPPER_IA_STATUS = "/pepper/ia/status"
    
    # Game topic for clock
    TOPIC_GAME_TIME = "/game/time"
    TOPIC_GAME_STATUS = "/game/status"

    def __init__(self, message_callback: Callable):
        self.client = mqtt.Client()
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message
        self.message_callback = message_callback
        
        # Mapping of status topics
        self.status_topics = {
            # Capteur lot
            self.TOPIC_BUTTON_STATUS: "button",
            self.TOPIC_SIMON_STATUS: "simon",
            self.TOPIC_TOF_STATUS: "tof",
            # Detection lot
            self.TOPIC_BICAMERA_STATUS: "bicamera",
            self.TOPIC_MASTERMIND_STATUS: "mastermind",
            self.TOPIC_EMOTIONS_STATUS: "emotions",
            self.TOPIC_PHOTO_STATUS: "photo",
            # IA lot
            self.TOPIC_TEXTE_STATUS: "texte",
            self.TOPIC_VOCALE_STATUS: "vocale",
            self.TOPIC_LABYRINTHE_STATUS: "labyrinthe",
            # pepper
            self.TOPIC_PEPPER_STATUS: "pepper_status",
            # game
            self.TOPIC_GAME_STATUS: "game_status"
        }
        
        # Mapping of command topics
        self.cmd_topics = {
            # Capteur lot
            "button": self.TOPIC_BUTTON_CMD,
            "simon": self.TOPIC_SIMON_CMD,
            "tof": self.TOPIC_TOF_CMD,
            # Detection lot
            "bicamera": self.TOPIC_BICAMERA_CMD,
            "mastermind": self.TOPIC_MASTERMIND_CMD,
            "emotions": self.TOPIC_EMOTIONS_CMD,
            "photo": self.TOPIC_PHOTO_CMD,
            # IA lot
            "texte": self.TOPIC_TEXTE_CMD,
            "vocale": self.TOPIC_VOCALE_CMD,
            "labyrinthe": self.TOPIC_LABYRINTHE_CMD,
            # pepper
            "pepper_cmd": self.TOPIC_PEPPER_CMD,
            # game
            "game_status": self.TOPIC_GAME_STATUS
        }

    def on_connect(self, client, userdata, flags, rc):
        """Called when the client connects to the broker."""
        print(f"Connected with result code {rc}")
        # Subscribe to all status topics
        for topic in self.status_topics.keys():
            client.subscribe(topic)
        client.subscribe(self.TOPIC_PEPPER_CMD)

    def on_message(self, client, userdata, msg):
        """Called when a message is received from the broker."""
        workshop_type = self.status_topics.get(msg.topic)
        if workshop_type:
            status = msg.payload.decode()
            self.message_callback(workshop_type, status)


    def clock_publish(self, time_str: str):
        """Publishes the time to the game time topic."""
        self.client.publish(self.TOPIC_GAME_TIME, time_str)
        #print(f"Time remaining: {time_str}")


    def run(self):
        """Connects to the broker and starts the loop to process messages."""
        try:
            self.client.connect("134.214.51.148", 1883, 60)
            self.client.loop_forever()
        except KeyboardInterrupt:
            print("Stopping MQTT client")
            self.client.disconnect()

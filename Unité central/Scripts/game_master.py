import threading
import time
from enum import Enum, auto
from mqtt_handler import MQTTHandler
from dataclasses import dataclass

class GameState(Enum):
    STANDBY = auto()
    PHASE_1 = auto()
    PHASE_2 = auto()
    PHASE_3 = auto()
    COMPLETED = auto()
    FAILED = auto()

@dataclass
class WorkshopStatus:
    button: str = "waiting"
    simon: str = "waiting"
    tof: str = "waiting"
    bicamera: str = "waiting"
    mastermind: str = "waiting"
    emotions: str = "waiting"
    photo: str = "waiting"
    texte: str = "waiting"
    vocale: str = "waiting"
    labyrinthe: str = "waiting"

class GameMaster:
    def __init__(self):
        self.state = GameState.STANDBY
        self.workshop_status = WorkshopStatus()
        self.mqtt_handler = MQTTHandler(self.handle_workshop_update)
        self.mqtt_handler.client.message_callback_add(MQTTHandler.TOPIC_PEPPER_CMD, self.handle_pepper_cmd)
        self.mqtt_handler.client.subscribe(MQTTHandler.TOPIC_PEPPER_CMD)
        self.timer_thread = None  # Référence au thread du timer
        self.timer_stop_event = threading.Event()  # Événement pour arrêter le timer
        print(f"Game initialized in STANDBY state")
        self.mqtt_handler.run()
        self.mqtt_handler.clock_publish("60:00")

    def start_timer(self):
        """Starts or resets a countdown timer of 1 hour."""
        if self.timer_thread and self.timer_thread.is_alive():
            # Arrêter le timer en cours
            print("Resetting timer...")
            self.timer_stop_event.set()
            self.timer_thread.join()

        # Réinitialiser l'événement d'arrêt
        self.timer_stop_event.clear()

        # Démarrer un nouveau timer
        def countdown():
            total_seconds = 3600  # 1 hour
            while total_seconds > 0 and not self.timer_stop_event.is_set() and self.state not in [GameState.COMPLETED, GameState.FAILED]:
                mins, secs = divmod(total_seconds, 60)
                time_str = f"{mins:02}:{secs:02}"
                self.mqtt_handler.clock_publish(time_str)
                time.sleep(1)
                total_seconds -= 1
            if not self.timer_stop_event.is_set() and self.state not in [GameState.COMPLETED, GameState.FAILED]:
                self.state = GameState.FAILED
                print("Time's up! Game FAILED!")
                self.mqtt_handler.clock_publish("60:00")

        self.timer_thread = threading.Thread(target=countdown, daemon=True)
        self.timer_thread.start()

    def handle_workshop_update(self, workshop_type: str, status: str):
        """Called when a message is received from any workshop"""
        print(f"Received update from {workshop_type}: {status}")
        
        if workshop_type == "game_status" and status == "in_game":
            print("Starting game...")
            self.start_game()

        setattr(self.workshop_status, workshop_type, status)
        
        # Check phase transitions based on workshop status
        match self.state:
            case GameState.PHASE_1:
                if (self.workshop_status.button == "finish" and 
                    self.workshop_status.simon == "finish" and 
                    self.workshop_status.tof == "finish"):
                    self.state = GameState.PHASE_2
                    self.mqtt_handler.client.publish(MQTTHandler.TOPIC_PEPPER_CAPTEUR_STATUS, "finish")
                    print("All capteur workshops finished - Moving to PHASE_2!")
                    
            case GameState.PHASE_2:
                if (self.workshop_status.bicamera == "finish" and 
                    self.workshop_status.mastermind == "finish" and 
                    self.workshop_status.emotions == "finish" and 
                    self.workshop_status.photo == "finish"):
                    self.state = GameState.PHASE_3
                    self.mqtt_handler.client.publish(MQTTHandler.TOPIC_PEPPER_DETECTION_STATUS, "finish")
                    print("All detection workshops finished - Moving to PHASE_3!")
                    
            case GameState.PHASE_3:
                if (self.workshop_status.texte == "finish" and 
                    self.workshop_status.vocale == "finish" and 
                    self.workshop_status.labyrinthe == "finish"):
                    self.state = GameState.COMPLETED
                    self.mqtt_handler.client.publish(MQTTHandler.TOPIC_PEPPER_IA_STATUS, "finish")
                    print("All IA workshops finished - Game COMPLETED!")

    def start_game(self):
        """Starts the game by moving to PHASE_1 and starting the timer"""
        self.state = GameState.PHASE_1
        print("Moving to PHASE_1")
        self.start_timer()
        # Start MQTT loop
        

    def reset(self):
        """Resets the game to its initial state."""
        print("Resetting game...")
    
        # Arrêter le timer en cours, s'il existe
        if self.timer_thread and self.timer_thread.is_alive():
            print("Stopping the timer...")
            self.timer_stop_event.set()
            self.timer_thread.join()
    
        # Réinitialiser l'état du jeu
        self.state = GameState.STANDBY
        self.mqtt_handler.clock_publish("60:00")  # Réinitialiser l'horloge MQTT
        self.workshop_status = WorkshopStatus()  # Réinitialiser l'état des ateliers
        self.mqtt_handler.client.publish(MQTTHandler.TOPIC_PEPPER_CAPTEUR_STATUS, "waiting")
        self.mqtt_handler.client.publish(MQTTHandler.TOPIC_PEPPER_DETECTION_STATUS, "waiting")
        self.mqtt_handler.client.publish(MQTTHandler.TOPIC_PEPPER_IA_STATUS, "waiting")

    def handle_pepper_cmd(self, client, userdata, msg):
        """Handles commands received on the /pepper/cmd topic."""
        print(f"Received {msg.payload.decode()}")
        command = msg.payload.decode()
        if command == "reset":
            self.reset()
    

def main():
    game = GameMaster()

if __name__ == "__main__":
    main()

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
        print(f"Game initialized in STANDBY state")
        
    def start_timer(self):
        """Starts a countdown timer of 1 hour."""
        def countdown():
            total_seconds = 3600  # 1 hour
            while total_seconds > 0 and self.state not in [GameState.COMPLETED, GameState.FAILED]:
                mins, secs = divmod(total_seconds, 60)
                time_str = f"{mins:02}:{secs:02}"
                self.mqtt_handler.clock_publish(time_str)
                time.sleep(1)
                total_seconds -= 1
            if self.state not in [GameState.COMPLETED, GameState.FAILED]:
                self.state = GameState.FAILED
                print("Time's up! Game FAILED!")
                self.mqtt_handler.clock_publish("00:00")

        timer_thread = threading.Thread(target=countdown, daemon=True)
        timer_thread.start()

    def handle_workshop_update(self, workshop_type: str, status: str):
        """Called when a message is received from any workshop"""
        print(f"Received update from {workshop_type}: {status}")
        
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
        self.mqtt_handler.run()

    def reset(self):
        """Resets the game to its initial state."""
        self.state = GameState.STANDBY

    def handle_pepper_cmd(self, client, userdata, msg):
        """Handles commands received on the /pepper/cmd topic."""
        command = msg.payload.decode()
        if command == "reset":
            self.reset()
    

def main():
    game = GameMaster()

    game_status=""

    def handle_game_status(workshop_type, status):
        nonlocal game_status
        if workshop_type == "game_status":
            game_status = (status == "in_game")

    game.mqtt_handler.message_callback = handle_game_status
    
    while True:
        if game_status == "in_game":
            print("Starting game...")
            game.start_game()
            break

if __name__ == "__main__":
    main()

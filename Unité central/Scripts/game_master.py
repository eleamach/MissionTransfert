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
                    print("All capteur workshops finished - Moving to PHASE_2!")
                    
            case GameState.PHASE_2:
                if (self.workshop_status.bicamera == "finish" and 
                    self.workshop_status.mastermind == "finish" and 
                    self.workshop_status.emotions == "finish" and 
                    self.workshop_status.photo == "finish"):
                    self.state = GameState.PHASE_3
                    print("All detection workshops finished - Moving to PHASE_3!")
                    
            case GameState.PHASE_3:
                if (self.workshop_status.texte == "finish" and 
                    self.workshop_status.vocale == "finish" and 
                    self.workshop_status.labyrinthe == "finish"):
                    self.state = GameState.COMPLETED
                    print("All IA workshops finished - Game COMPLETED!")

    def start_game(self):
        """Starts the game by moving to PHASE_1 and starting the timer"""
        self.state = GameState.PHASE_1
        print("Moving to PHASE_1")
        # Reset all workshops and start timer
        self.mqtt_handler.reset_workshops()
        self.start_timer()
        # Start MQTT loop
        self.mqtt_handler.run()

def main():
    game = GameMaster()
    
    while True:
        user_input = input("Do you want to start the game? (o/n): ").lower()
        if user_input == 'o':
            print("Starting game...")
            game.start_game()
            break
        elif user_input == 'n':
            print("Game start cancelled")
            break
        else:
            print("Invalid input. Please enter 'o' for yes or 'n' for no.")

if __name__ == "__main__":
    main()

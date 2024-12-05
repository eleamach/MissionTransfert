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
    megaming: str = "waiting"
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
        self.state = GameState.PHASE_1
        print(f"Game starting in PHASE_1")
        print(f"Initial workshop status: button={self.workshop_status.button}, "
              f"simon={self.workshop_status.simon}, tof={self.workshop_status.tof}")

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
                    self.workshop_status.megaming == "finish" and 
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
        """Starts the MQTT handler which will publish initial states"""
        print(f"Current state: {self.state}")
        self.mqtt_handler.run()

def main():
    game = GameMaster()
    game.start_game()

if __name__ == "__main__":
    main()

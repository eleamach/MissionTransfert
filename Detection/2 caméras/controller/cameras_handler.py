import cv2
import time
import torch
from ultralytics import YOLO
import paho.mqtt.client as mqtt
import threading

# Configuration
CAMERAS = {
    "cam1": {
        "url": "http://10.51.11.59:81/stream",
        "target": 4,
        "topic": "/detection/bi-camera/cam1-msg",
        "window_name": "Camera 1"
    },
    "cam2": {
        "url": "http://10.51.11.65:81/stream",
        "target": 2,
        "topic": "/detection/bi-camera/cam2-msg",
        "window_name": "Camera 2"
    }
}

MQTT_BROKER = "134.214.51.148"
CMD_TOPIC = "/detection/bi-camera/cmd"

# Variables globales pour le statut
target_reached_time = {cam: 0 for cam in CAMERAS}
success_state = False
reset_lock = threading.Lock()

# Callbacks MQTT
def on_connect(client, userdata, flags, reason_code, properties=None):
    if reason_code == 0:
        print("Connecté au broker MQTT")
        client.subscribe(CMD_TOPIC)
    else:
        print(f"Échec de connexion au broker MQTT, code retour={reason_code}")

def on_disconnect(client, userdata, rc):
    print("Déconnecté du broker MQTT")

def on_message(client, userdata, msg):
    global success_state
    if msg.topic == CMD_TOPIC and msg.payload.decode() == "reset":
        with reset_lock:
            success_state = False
            for cam in CAMERAS:
                target_reached_time[cam] = 0
        print("État réinitialisé")

def process_camera(cam_id, cam_config, model, client):
    global success_state
    
    cap = cv2.VideoCapture(cam_config["url"])
    if not cap.isOpened():
        print(f"Erreur : Impossible d'ouvrir le flux vidéo pour {cam_id}")
        return

    cv2.namedWindow(cam_config["window_name"], cv2.WINDOW_NORMAL)
    previous_state = None  # Pour tracker les changements d'état

    while True:
        ret, frame = cap.read()
        if not ret:
            print(f"Erreur : Impossible de lire l'image de {cam_id}")
            break

        # Effectuer la détection avec YOLO
        results = model.predict(frame, conf=0.5)
        
        # Compter le nombre de personnes
        num_persons = sum(1 for r in results for c in r.boxes.cls if int(c) == 0)

        # Vérifier si l'objectif est atteint
        with reset_lock:
            current_time = time.time()
            if num_persons == cam_config["target"]:
                if target_reached_time[cam_id] == 0:
                    target_reached_time[cam_id] = current_time
            else:
                target_reached_time[cam_id] = 0

            # Vérifier si les deux caméras ont atteint leur objectif pendant 3 secondes
            all_targets_reached = all(
                t > 0 and current_time - t >= 3 
                for t in target_reached_time.values()
            )
            
            if all_targets_reached:
                success_state = True

        # Envoyer le message MQTT si l'état change
        if client:
            current_state = "Bravo" if success_state else f"Humains:{num_persons}"
            if current_state != previous_state:
                try:
                    client.publish(cam_config["topic"], current_state)
                    print(f"Message MQTT envoyé pour {cam_id}: {current_state}")
                    previous_state = current_state
                except Exception as e:
                    print(f"Erreur d'envoi MQTT pour {cam_id}: {e}")

        # Préparer l'affichage
        annotated_frame = results[0].plot()
        
        # Afficher le texte approprié
        if success_state:
            cv2.putText(annotated_frame, "Bravo !!", (10, 30),
                       cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
            cv2.putText(annotated_frame, "Rouge 1", (10, 70),
                       cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
        else:
            cv2.putText(annotated_frame, f"Personnes: {num_persons}/{cam_config['target']}", 
                       (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)

        cv2.imshow(cam_config["window_name"], annotated_frame)

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

        time.sleep(0.01)

    cap.release()
    cv2.destroyWindow(cam_config["window_name"])

def main():
    # Initialisation du client MQTT
    try:
        client = mqtt.Client(protocol=mqtt.MQTTv5)
        client.on_connect = on_connect
        client.on_disconnect = on_disconnect
        client.on_message = on_message
        
        print("Tentative de connexion au broker MQTT...")
        client.connect(MQTT_BROKER, 1883, keepalive=60)
        client.loop_start()
    except Exception as e:
        print(f"Erreur de connexion MQTT: {e}")
        print("Continuation sans MQTT...")
        client = None

    # Création d'un détecteur YOLO
    model = YOLO("yolov8n.pt")

    # Créer et démarrer les threads pour chaque caméra
    threads = []
    for cam_id, cam_config in CAMERAS.items():
        thread = threading.Thread(
            target=process_camera,
            args=(cam_id, cam_config, model, client)
        )
        thread.start()
        threads.append(thread)

    # Attendre que tous les threads se terminent
    for thread in threads:
        thread.join()

    # Nettoyage
    cv2.destroyAllWindows()
    if client:
        client.loop_stop()
        client.disconnect()

if __name__ == "__main__":
    main()
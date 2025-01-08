import cv2
import time
import torch
from ultralytics import YOLO
import paho.mqtt.client as mqtt
from collections import deque

# Configuration
ESP32_STREAM_URL = "http://10.51.11.59:81/stream"
MQTT_BROKER = "134.214.51.148"
MQTT_TOPIC = "/detection/bi-camera/cam1-msg"
MQTT_CMD_TOPIC = "/detection/bi-camera/cmd"

DETECTION_TIME_THRESHOLD = 5.0  #La détection des personnes doit durer pendant 2 secondes avant validation

# Variable globale pour le mode reset
waiting_for_reset = False

# Callbacks MQTT
def on_connect(client, userdata, flags, rc, properties=None):
    if rc == 0:
        print("Connecté au broker MQTT")
        client.subscribe(MQTT_CMD_TOPIC)
    else:
        print(f"Échec de connexion au broker MQTT, code retour={rc}")

def on_disconnect(client, userdata, rc, properties=None):
    print("Déconnecté du broker MQTT")

def on_message(client, userdata, message):
    global waiting_for_reset
    if message.topic == MQTT_CMD_TOPIC:
        if message.payload.decode() == "reset":
            waiting_for_reset = False
            print("Reset reçu - reprise de la détection")

def main():
    global waiting_for_reset
    try:
        client = mqtt.Client(protocol=mqtt.MQTTv5)
        client.on_connect = on_connect
        client.on_disconnect = on_disconnect
        client.on_message = on_message
        print("Tentative de connexion au broker MQTT...")
        client.connect(MQTT_BROKER, 1883, keepalive=60)
        client.loop_start()
        client.subscribe(MQTT_CMD_TOPIC)
    except Exception as e:
        print(f"Erreur de connexion MQTT: {e}")
        print("Continuation sans MQTT...")
        client = None

    cap = cv2.VideoCapture(ESP32_STREAM_URL)

    if not cap.isOpened():
        print("Erreur : Impossible d'ouvrir le flux vidéo")
        if client:
            client.loop_stop()
            client.disconnect()
        return

    # Création d'un détecteur YOLO
    model = YOLO("yolov8n.pt")
    
    # Création d'une file pour la moyenne glissante
    detection_buffer = deque(maxlen=6)
    previous_avg_persons = -1

    # Ajouter ces variables pour le suivi temporel
    two_persons_start_time = None
    message_sent = False

    # Créer la fenêtre une seule fois avant la boucle
    cv2.namedWindow('Flux vidéo ESP32', cv2.WINDOW_NORMAL)

    while True:
        ret, frame = cap.read()

        if not ret:
            print("Erreur : Impossible de lire l'image")
            break

        # Effectuer la détection avec YOLO
        results = model.predict(frame, conf=0.5)
        
        # Compter le nombre de personnes
        num_persons = 0
        for r in results:
            for c in r.boxes.cls:
                if int(c) == 0:  # 0 est l'ID de la classe "person"
                    num_persons += 1

        # Ajouter la détection courante au buffer
        detection_buffer.append(num_persons)
        
        # Calculer la moyenne arrondie
        avg_persons = round(sum(detection_buffer) / len(detection_buffer))

        # Si on n'attend pas un reset, on peut envoyer des messages
        if not waiting_for_reset:
            if avg_persons == 2:
                if two_persons_start_time is None:
                    two_persons_start_time = time.time()
                elif time.time() - two_persons_start_time >= DETECTION_TIME_THRESHOLD:
                    try:
                        if client:
                            client.publish(MQTT_TOPIC, "Bravo\nRouge 6")
                            print("Message spécial envoyé: Bravo\\nRouge 6")
                            waiting_for_reset = True  # On attend maintenant un reset
                    except Exception as e:
                        print(f"Erreur d'envoi MQTT: {e}")
            else:
                two_persons_start_time = None

            # Envoyer le message MQTT du nombre de personnes
            if client and avg_persons != previous_avg_persons:
                try:
                    mqtt_message = f"Humains:{avg_persons}"
                    client.publish(MQTT_TOPIC, mqtt_message)
                    print(f"Message MQTT envoyé: {mqtt_message}")
                    previous_avg_persons = avg_persons
                except Exception as e:
                    print(f"Erreur d'envoi MQTT: {e}")

        # Modifier l'affichage pour montrer la moyenne
        annotated_frame = results[0].plot()
        cv2.putText(annotated_frame, f"Personnes (moy): {avg_persons}", (10, 30),
                    cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)

        # Mettre à jour l'affichage dans la fenêtre existante
        cv2.imshow('Flux vidéo ESP32', annotated_frame)

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

        time.sleep(0.01)

    # Nettoyage
    cap.release()
    cv2.destroyAllWindows()
    if client:
        client.loop_stop()
        client.disconnect()

if __name__ == "__main__":
    main()
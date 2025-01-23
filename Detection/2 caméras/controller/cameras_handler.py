import cv2
import time
import torch
from ultralytics import YOLO
import paho.mqtt.client as mqtt
from collections import deque

# Configuration
ESP32_STREAM_URL_1 = "http://10.51.11.59:81/stream"
ESP32_STREAM_URL_2 = "http://10.51.11.65:81/stream"
MQTT_BROKER = "134.214.51.148"
MQTT_TOPIC_CAM1 = "/detection/bi-camera/cam1-msg"
MQTT_TOPIC_CAM2 = "/detection/bi-camera/cam2-msg"
MQTT_CMD_TOPIC = "/detection/bi-camera/cmd"

DETECTION_TIME_THRESHOLD = 2.0

# Variables globales
waiting_for_reset = False
cam1_persons = 0
cam2_persons = 0

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
    print(f"Message reçu sur {message.topic}: {message.payload.decode()}")
    if message.topic == MQTT_CMD_TOPIC:
        if message.payload.decode() == "reset":
            waiting_for_reset = False
            print("Reset reçu - reprise de la détection")

def process_frame(frame, model):
    results = model.predict(frame, conf=0.5)
    num_persons = 0
    for r in results:
        for c in r.boxes.cls:
            if int(c) == 0:  # 0 est l'ID de la classe "person"
                num_persons += 1
    return results, num_persons

def main():
    global waiting_for_reset, cam1_persons, cam2_persons
    
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

    # Initialisation des deux caméras
    cap1 = cv2.VideoCapture(ESP32_STREAM_URL_1)
    cap2 = cv2.VideoCapture(ESP32_STREAM_URL_2)

    if not cap1.isOpened() or not cap2.isOpened():
        print("Erreur : Impossible d'ouvrir un ou les flux vidéo")
        if client:
            client.loop_stop()
            client.disconnect()
        return

    model = YOLO("yolov8n.pt")
    
    # Buffers pour les deux caméras
    detection_buffer1 = deque(maxlen=6)
    detection_buffer2 = deque(maxlen=6)
    previous_avg_persons1 = -1
    previous_avg_persons2 = -1

    # Variables de timing pour les deux caméras
    condition_start_time = None

    cv2.namedWindow('Camera 1', cv2.WINDOW_NORMAL)
    cv2.namedWindow('Camera 2', cv2.WINDOW_NORMAL)

    while True:
        ret1, frame1 = cap1.read()
        ret2, frame2 = cap2.read()

        if not ret1 or not ret2:
            print("Erreur : Impossible de lire les images")
            break

        # Traitement caméra 1
        results1, num_persons1 = process_frame(frame1, model)
        detection_buffer1.append(num_persons1)
        avg_persons1 = round(sum(detection_buffer1) / len(detection_buffer1))

        # Traitement caméra 2
        results2, num_persons2 = process_frame(frame2, model)
        detection_buffer2.append(num_persons2)
        avg_persons2 = round(sum(detection_buffer2) / len(detection_buffer2))

        # Mise à jour des variables globales
        cam1_persons = avg_persons1
        cam2_persons = avg_persons2

        # Vérification des conditions (2 personnes sur cam1, 3 sur cam2)
        if not waiting_for_reset:
            if avg_persons1 == 2 and avg_persons2 == 3:
                if condition_start_time is None:
                    condition_start_time = time.time()
                elif time.time() - condition_start_time >= DETECTION_TIME_THRESHOLD:
                    try:
                        if client:
                            client.publish(MQTT_TOPIC_CAM1, "Bravo\Bleu 6")
                            print("Message spécial envoyé: Bravo\\Bleu 6")
                            waiting_for_reset = True
                    except Exception as e:
                        print(f"Erreur d'envoi MQTT: {e}")
            else:
                condition_start_time = None

            # Envoi des mises à jour pour chaque caméra
            if client:
                if avg_persons1 != previous_avg_persons1:
                    client.publish(MQTT_TOPIC_CAM1, f"{avg_persons1}/2")
                    previous_avg_persons1 = avg_persons1
                if avg_persons2 != previous_avg_persons2:
                    client.publish(MQTT_TOPIC_CAM2, f"{avg_persons2}/3")
                    previous_avg_persons2 = avg_persons2

        # Affichage
        annotated_frame1 = results1[0].plot()
        annotated_frame2 = results2[0].plot()
        
        cv2.putText(annotated_frame1, f"Personnes (moy): {avg_persons1}", (10, 30),
                    cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
        cv2.putText(annotated_frame2, f"Personnes (moy): {avg_persons2}", (10, 30),
                    cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)

        cv2.imshow('Camera 1', annotated_frame1)
        cv2.imshow('Camera 2', annotated_frame2)

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

        time.sleep(0.005)

    # Nettoyage
    cap1.release()
    cap2.release()
    cv2.destroyAllWindows()
    if client:
        client.loop_stop()
        client.disconnect()

if __name__ == "__main__":
    main()
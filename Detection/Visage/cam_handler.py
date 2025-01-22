import cv2
import face_recognition
import os
import time
import paho.mqtt.client as mqtt

# Charger plusieurs images de la personne connue
known_encodings = []
known_names = []

# Ajouter plusieurs images de référence
for filename in os.listdir("known_person"):
    if filename.endswith(".jpg") or filename.endswith(".png"):
        image = face_recognition.load_image_file(f"known_person/{filename}")
        encodings = face_recognition.face_encodings(image)
        if encodings:
            known_encodings.append(encodings[0])
            known_names.append("Raphael")  # Nom associé à l'encodage

# URL du flux vidéo (TTGO-CAM)
stream_url = "http://10.51.11.60:81/stream"
cap = cv2.VideoCapture(stream_url)

if not cap.isOpened():
    print("Impossible d'ouvrir le flux vidéo")
    exit()

# Variables pour le chronomètre et la détection
start_time = None
required_name = "Raphael"  # La personne spécifique à détecter
time_threshold = 5  # Temps à maintenir (en secondes)
game_won = False  # Indique si le jeu est gagné
reset_requested = False  # Indique si une réinitialisation est en attente

# MQTT Configuration
BROKER = "134.214.51.148"
PORT = 1883
STATUS_TOPIC = "/detection/photo/status"
CMD_TOPIC = "/detection/photo/cmd"

# Initialiser le client MQTT
client = mqtt.Client()

def on_message(client, userdata, msg):
    """Callback pour traiter les messages MQTT reçus."""
    global game_won, start_time, reset_requested
    topic = msg.topic
    payload = msg.payload.decode()

    print(f"[MQTT] Reçu sur {topic}: {payload}")

    if topic == CMD_TOPIC and payload.lower() == "reset":
        print("[INFO] Réinitialisation demandée via MQTT.")
        reset_requested = True  # Activer le flag de réinitialisation

def reset_game():
    """Réinitialise le jeu et ses variables."""
    global game_won, start_time, reset_requested
    game_won = False
    start_time = None
    reset_requested = False
    print("[INFO] Jeu réinitialisé.")
    # Afficher l'état "waiting" via MQTT
    client.publish(STATUS_TOPIC, "waiting")
    print("[MQTT] Message 'waiting' envoyé.")

def draw_1_on_frame(frame):
    """Dessine un '1' rose sur l'image."""
    h, w, _ = frame.shape
    color = (255, 0, 255)  # Rose (BGR format)
    thickness = 5

    # Définir les coordonnées des lignes pour former un "1"
    vertical_line = [(w // 2, h // 4), (w // 2, 3 * h // 4)]
    base_line = [(w // 2 - 20, 3 * h // 4), (w // 2 + 20, 3 * h // 4)]
    diagonal_line = [(w // 2, h // 4), (w // 2 - 20, h // 4 + 20)]  # Ligne diagonale

    # Dessiner les lignes
    cv2.line(frame, vertical_line[0], vertical_line[1], color, thickness)  # Ligne verticale
    cv2.line(frame, base_line[0], base_line[1], color, thickness)  # Ligne horizontale en bas
    cv2.line(frame, diagonal_line[0], diagonal_line[1], color, thickness)  # Ligne diagonale

# Connexion au broker MQTT
client.on_message = on_message
client.connect(BROKER, PORT)
client.subscribe(CMD_TOPIC)
client.loop_start()

while True:
    ret, frame = cap.read()
    if not ret:
        print("Erreur de réception d'image")
        break

    # Vérifier si un reset est demandé
    if reset_requested:
        reset_game()
        continue

    if game_won:
        # Afficher un "1" rose sur l'écran
        draw_1_on_frame(frame)
        cv2.imshow("Flux vidéo", frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
        continue

    # Convertir l'image pour `face_recognition`
    rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    face_locations = face_recognition.face_locations(rgb_frame)
    face_encodings = face_recognition.face_encodings(rgb_frame, face_locations)

    # Vérifier si des visages correspondent à ceux connus
    detected_name = None
    for face_encoding in face_encodings:
        matches = face_recognition.compare_faces(known_encodings, face_encoding)
        if True in matches:
            first_match_index = matches.index(True)
            detected_name = known_names[first_match_index]
            break

    # Gestion du chronomètre
    if detected_name == required_name:
        if start_time is None:
            start_time = time.time()  # Démarrer le chronomètre
        elapsed_time = time.time() - start_time

        # Afficher le chronomètre
        cv2.putText(frame, f"Temps: {elapsed_time:.1f} s", (10, 50),
                    cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)

        # Vérifier la victoire
        if elapsed_time >= time_threshold:
            game_won = True
            start_time = None  # Réinitialiser le chronomètre
            # Envoyer "finish" via MQTT
            client.publish(STATUS_TOPIC, "finish")
            print("[MQTT] Message 'finish' envoyé.")
            continue
    else:
        start_time = None  # Réinitialiser si la bonne personne n'est pas détectée

        if detected_name:
            # Si une autre personne est détectée
            cv2.putText(frame, f"Personne incorrecte: {detected_name}", (10, 50),
                        cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2)
        else:
            # Aucun visage détecté
            cv2.putText(frame, "Personne non detectee!", (10, 50),
                        cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2)

    # Afficher le flux vidéo
    cv2.imshow("Flux vidéo", frame)

    # Quitter avec la touche 'q'
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
client.loop_stop()
import cv2
import face_recognition
import os
from collections import deque

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

# Définir une file pour mémoriser les dernières détections
detection_history = deque(maxlen=5)  # Mémorise les 5 dernières frames

while True:
    ret, frame = cap.read()
    if not ret:
        print("Erreur de réception d'image")
        break

    # Convertir l'image pour `face_recognition`
    rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    face_locations = face_recognition.face_locations(rgb_frame)
    face_encodings = face_recognition.face_encodings(rgb_frame, face_locations)

    # Stocker les détections actuelles
    frame_detections = []

    # Vérifier si des visages correspondent à ceux connus
    for (top, right, bottom, left), face_encoding in zip(face_locations, face_encodings):
        matches = face_recognition.compare_faces(known_encodings, face_encoding)
        name = "Inconnu"

        # Si une correspondance est trouvée, récupérer le nom
        if True in matches:
            first_match_index = matches.index(True)
            name = known_names[first_match_index]

        frame_detections.append((name, (top, right, bottom, left)))

    # Ajouter les détections de la frame à la file
    detection_history.append(frame_detections)

    # Calculer la moyenne des détections sur les frames récentes
    aggregated_detections = {}
    for history_frame in detection_history:
        for name, bbox in history_frame:
            if name not in aggregated_detections:
                aggregated_detections[name] = []
            aggregated_detections[name].append(bbox)

    # Afficher les détections moyennées sur l'image actuelle
    for name, bboxes in aggregated_detections.items():
        # Moyenne des coordonnées des boîtes englobantes
        avg_top = int(sum(bbox[0] for bbox in bboxes) / len(bboxes))
        avg_right = int(sum(bbox[1] for bbox in bboxes) / len(bboxes))
        avg_bottom = int(sum(bbox[2] for bbox in bboxes) / len(bboxes))
        avg_left = int(sum(bbox[3] for bbox in bboxes) / len(bboxes))

        # Définir la couleur selon le nom
        color = (0, 255, 0) if name == "Raphael" else (0, 0, 255)

        # Dessiner la boîte englobante et le nom
        cv2.rectangle(frame, (avg_left, avg_top), (avg_right, avg_bottom), color, 2)
        cv2.putText(frame, name, (avg_left, avg_top - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.9, color, 2)

    # Afficher le flux vidéo
    cv2.imshow("Flux vidéo", frame)

    # Quitter avec la touche 'q'
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
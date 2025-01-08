import cv2
import face_recognition
import os

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
            known_names.append("Personne connue")  # Nom associé à l'encodage

# URL du flux vidéo (TTGO-CAM)
stream_url = "http://10.51.11.60:81/stream"
cap = cv2.VideoCapture(stream_url)

if not cap.isOpened():
    print("Impossible d'ouvrir le flux vidéo")
    exit()

while True:
    ret, frame = cap.read()
    if not ret:
        print("Erreur de réception d'image")
        break

    # Convertir l'image pour `face_recognition`
    rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    face_locations = face_recognition.face_locations(rgb_frame)
    face_encodings = face_recognition.face_encodings(rgb_frame, face_locations)

    # Vérifier si des visages correspondent à ceux connus
    for (top, right, bottom, left), face_encoding in zip(face_locations, face_encodings):
        matches = face_recognition.compare_faces(known_encodings, face_encoding)
        name = "Inconnu"

        # Si une correspondance est trouvée, récupérer le nom
        if True in matches:
            first_match_index = matches.index(True)
            name = known_names[first_match_index]

        # Dessiner un rectangle autour du visage
        cv2.rectangle(frame, (left, top), (right, bottom), (0, 255, 0), 2)
        cv2.putText(frame, name, (left, top - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.9, (0, 255, 0), 2)

    # Afficher le flux vidéo
    cv2.imshow("Flux vidéo", frame)

    # Quitter avec la touche 'q'
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
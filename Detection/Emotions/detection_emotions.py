from roboflow import Roboflow
import supervision as sv
import cv2
import numpy as np
import random
import time

from src.emotion_validator import EmotionValidator
from src.mqtt_message import start_mqtt_service

mqtt_service = start_mqtt_service(on_cmd_receive=lambda cmd: cmd_receive(cmd))

# Initialisation de l'API Roboflow
rf = Roboflow(api_key="DPxWxOvyi036rO9Y7uKQ")
project = rf.workspace().project("emotion-detection-cwq4g")
model = project.version(1).model

# Capture du flux vidéo (0 pour la caméra par défaut)
capture = cv2.VideoCapture(1, cv2.CAP_V4L2)

capture.set(cv2.CAP_PROP_FRAME_WIDTH, 320)
capture.set(cv2.CAP_PROP_FRAME_HEIGHT, 240)
capture.set(cv2.CAP_PROP_FPS, 30)

if not capture.isOpened():
    print("Erreur : Impossible d'accéder à la caméra.")
    exit()

# Charger des images (emojis) à afficher
check = cv2.imread("images/verifier.png", cv2.IMREAD_UNCHANGED) 
empty_check = cv2.imread("images/carre.png", cv2.IMREAD_UNCHANGED)    
happy = cv2.imread("images/happy.png", cv2.IMREAD_UNCHANGED) 
happy_valide = cv2.imread("images/happy_valide.png", cv2.IMREAD_UNCHANGED)
sad = cv2.imread("images/sad.png", cv2.IMREAD_UNCHANGED) 
sad_valide = cv2.imread("images/sad_valide.png", cv2.IMREAD_UNCHANGED)
surprise = cv2.imread("images/surprise.png", cv2.IMREAD_UNCHANGED) 
surprise_valide = cv2.imread("images/surprise_valide.png", cv2.IMREAD_UNCHANGED)
angry = cv2.imread("images/angry.png", cv2.IMREAD_UNCHANGED) 
angry_valide = cv2.imread("images/angry_valide.png", cv2.IMREAD_UNCHANGED)

check_resized = cv2.resize(check, (150, 150), interpolation=cv2.INTER_AREA)
empty_check_resized = cv2.resize(empty_check, (150, 150), interpolation=cv2.INTER_AREA)
happy_resized = cv2.resize(happy, (150, 150), interpolation=cv2.INTER_AREA)
happy_valide_resized = cv2.resize(happy_valide, (150, 150), interpolation=cv2.INTER_AREA)
sad_resized = cv2.resize(sad, (150, 150), interpolation=cv2.INTER_AREA)
sad_valide_resized = cv2.resize(sad_valide, (150, 150), interpolation=cv2.INTER_AREA)
surprise_resized = cv2.resize(surprise, (150, 150), interpolation=cv2.INTER_AREA)
surprise_valide_resized = cv2.resize(surprise_valide, (150, 150), interpolation=cv2.INTER_AREA)
angry_resized = cv2.resize(angry, (150, 150), interpolation=cv2.INTER_AREA)
angry_valide_resized = cv2.resize(angry_valide, (150, 150), interpolation=cv2.INTER_AREA)


validator = EmotionValidator()

# Fonction pour superposer une image avec transparence (RGBA)
def overlay_image(background, overlay, x, y):
    """Superpose une image avec transparence sur un fond."""
    h, w, _ = overlay.shape
    alpha = overlay[:, :, 3] / 255.0  # Canal alpha normalisé
    for c in range(0, 3):  # Boucle sur les canaux (BGR)
        background[y:y+h, x:x+w, c] = (alpha * overlay[:, :, c] +
                                       (1 - alpha) * background[y:y+h, x:x+w, c])

def show_text(res):
    text_frame = np.ones((600, 900, 3), dtype=np.uint8)* 255
    if "/" in res:
        info = res.split("/")
        level = info[0]
        detected = eval(info[1])
        
        # Ajouter un titre
        cv2.putText(text_frame, level, (10, 40), cv2.FONT_HERSHEY_SIMPLEX, 1.0, (0, 255, 0), 2, cv2.LINE_AA)

        if level == "1":
            pos = 10
            for i in range (3):
                if i + 1 <= len(detected):
                    overlay_image(text_frame, check_resized, pos, 80)
                else:
                    overlay_image(text_frame, empty_check_resized, pos, 80)
                pos += 150

            if len(detected) == 3:
                time.sleep(1)
        
        elif level == "2":
            if not detected:
                overlay_image(text_frame, happy_resized, 50, 80)
                overlay_image(text_frame, sad_resized, 200, 80)
            else:
                if len(detected[0])< 2:
                    if "Happy" in detected[0]:
                        overlay_image(text_frame, happy_valide_resized, 50, 80)
                    else:
                        overlay_image(text_frame, happy_resized, 50, 80)
                    if "Sad" in detected[0]:
                        overlay_image(text_frame, sad_valide_resized, 250, 80)
                    else:
                        overlay_image(text_frame, sad_resized, 250, 80)
                elif "Happy" in detected[0] and "Sad" in detected[0]:
                    if len(detected)<2:
                        overlay_image(text_frame, happy_valide_resized, 50, 80)
                        overlay_image(text_frame, sad_valide_resized, 250, 80)
                        time.sleep(1)
                    else :
                        if "Surprise" in detected[1]:
                            overlay_image(text_frame, surprise_valide_resized, 50, 80)
                        else:
                            overlay_image(text_frame, surprise_resized, 50, 80)
                        if "Angry" in detected[1]:
                            overlay_image(text_frame, angry_valide_resized , 250, 80)
                        else:
                            overlay_image(text_frame, angry_resized, 250, 80)
        elif level == "3":
            story_text = [
                "Le createur se tenait devant son oeuvre inachevee.",
                "Les outils etaient eparpilles autour de lui, temoins silencieux de nuits sans sommeil.",
                "En observant son travail, il sentit une profonde melancolie, comme si tout etait perdu.",
                "Il avait tant espere que cette invention changerait le monde.",
                "Les murmures autour de lui firent bouillir son esprit.",
                "Des voix critiques, pleines de jugements, resonnaient dans sa tete.",
                "Il essaya de se concentrer, mais une vision devant ses yeux le remplit de degout.",
                "Malgre tout, il serra les poings. Il devait aller jusqu'au bout, peu importe les obstacles."
            ]
            y_offset = 70
            for line in story_text:
                cv2.putText(text_frame, line, (20, y_offset), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 0, 0), 1, cv2.LINE_AA)
                y_offset += 30
            pos = 10
            for i in range (3):
                if i + 1 <= len(detected):
                    overlay_image(text_frame, check_resized, pos, 300)
                else:
                    overlay_image(text_frame, empty_check_resized, pos, 300)
                pos += 150

            if len(detected) == 3:
                time.sleep(1)
    else:
        if res == "Fin":
            cv2.putText(text_frame, "4", (370, 350), cv2.FONT_HERSHEY_SIMPLEX, 8.0, (67, 198, 231), 8, cv2.LINE_AA)
            data = "finish"
            mqtt_service.publish(data, "/detection/emotions/status")
        elif res == "Attente":
            cv2.putText(text_frame, "4", (370, 350), cv2.FONT_HERSHEY_SIMPLEX, 8.0, (67, 198, 231), 8, cv2.LINE_AA)

            

    cv2.imshow("Fenêtre Texte", text_frame)

def cmd_receive(cmd):
    if cmd == "reset":
        validator.reset()
        data = "waiting"
        mqtt_service.publish(data, "/detection/emotions/status")

# Créer les fenêtres
cv2.namedWindow("Flux vidéo annoté", cv2.WINDOW_NORMAL)
cv2.resizeWindow("Flux vidéo annoté", 960, 720)

cv2.namedWindow("Fenêtre Texte", cv2.WINDOW_NORMAL)
cv2.resizeWindow("Fenêtre Texte", 900, 600)

# Création des annotateurs
bounding_box_annotator = sv.BoxAnnotator()
label_annotator = sv.LabelAnnotator()

frame_count = 0

# Variable pour stocker les émotions détectées
detected_emotions = []
level = 1

while True:
    ret, frame = capture.read()

    if not ret or frame is None:
        print("Erreur : Impossible de lire le flux de la caméra.")
        continue

    frame_count += 1
    if frame_count % 5 != 0:
        cv2.imshow("Flux vidéo annoté", frame)
        continue

    # Prédiction sur la frame
    result = model.predict(frame, confidence=0.4, overlap=0.3).json()
    
    # Extraire les émotions détectées
    labels = [item["class"] for item in result["predictions"]]
    
    # Stocker les émotions dans la variable
    detected_emotions = labels 
    res = validator.validateLevel(detected_emotions)

    show_text(res)

    detections = sv.Detections.from_inference(result)

    # Annotation du flux vidéo
    annotated_frame = bounding_box_annotator.annotate(scene=frame, detections=detections)
    annotated_frame = label_annotator.annotate(scene=annotated_frame, detections=detections, labels=labels)
    larger_frame = cv2.resize(annotated_frame, (960, 720))  # Redimensionner pour l'affichage


    # Afficher les fenêtres
    cv2.imshow("Flux vidéo annoté", larger_frame)
    

    # Quitter avec la touche ESC
    if cv2.waitKey(1) & 0xFF == 27:
        break

# Libérer les ressources
capture.release()
cv2.destroyAllWindows()
mqtt_service.stop()

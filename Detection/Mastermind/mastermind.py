#!/usr/bin/env python

import cv2
import numpy as np
import time 
import random

import importlib
import mqtt_message
importlib.reload(mqtt_message)

from mqtt_message import start_mqtt_service

# For OpenCV2 image display
WINDOW_NAME = 'Mastermind'

class MastermindGame:
    def __init__(self, all_colors):
        self.all_colors = all_colors
        self.essais = 0
        self.essais_max = 15
        self.goals = self.random_goal()
        self.correcte = 0
        self.malPlace = 0
        self.backup_detected_colors = ['', '', '', '']
        self.color_ranges = self.create_color_ranges()

    def create_color_ranges(self):
        """Créer un dictionnaire avec les plages de couleurs HSV pour chaque couleur."""
        color_ranges = {}
        for color in self.all_colors:
            lower_color, upper_color = color_name_to_color_code(color)
            if lower_color is not None and upper_color is not None:
                color_ranges[color] = (lower_color, upper_color)
        return color_ranges

    def random_goal(self):
        """Génère un nouvel objectif aléatoire de 4 couleurs."""
        new_goals = random.sample(self.all_colors, 4)
        print(f"Nouvel objectif : {new_goals}")
        return new_goals

    def reset_game(self):
        """Réinitialise le jeu."""
        self.essais = 0
        self.correcte = 0
        self.malPlace = 0
        self.goals = self.random_goal()
        print("Jeu réinitialisé.")
        data = {'correcte': 0, 'mal_place': 0, 'essais' : ""}
        mqtt_service.publish(data, "/detection/mastermind/led")
        mqtt_service.publish("waiting", "/detection/mastermind/status")

    def update_results(self, detected_colors):
        """Met à jour les résultats après chaque essai."""
        self.correcte = 0
        self.malPlace = 0
        for index, goal in enumerate(self.goals):
            if goal == detected_colors[index]:
                self.correcte += 1
            elif goal in detected_colors:
                self.malPlace += 1
        print(f"Correcte : {self.correcte} Mal placé : {self.malPlace}")


def track(image, color_ranges, rois):
    if image is None:
        print("Erreur : image vide.")
        return [None, None, None, None]

    # Blur the image to reduce noise
    blur = cv2.GaussianBlur(image, (5, 5), 0)

    # Convert BGR to HSV
    hsv = cv2.cvtColor(blur, cv2.COLOR_BGR2HSV)

    detected_colors = []

    for roi in rois:
        # Extraire l'image de la région d'intérêt (ROI)
        roi_image = hsv[roi[1]:roi[3], roi[0]:roi[2]]

        if roi_image.size == 0:
            print(f"Erreur : ROI vide pour {roi}.")
            detected_colors.append(None)
            continue

        colors_found = []  # Liste temporaire pour chaque ROI

        for color, (lower_color, upper_color) in color_ranges.items():
            mask = cv2.inRange(roi_image, lower_color, upper_color)

            # Blur the mask and find contours
            bmask = cv2.GaussianBlur(mask, (5, 5), 0)
            contours, _ = cv2.findContours(bmask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

            for contour in contours:
                if cv2.contourArea(contour) > 4000:  # Minimum area to consider for detection
                    colors_found.append(color)

        # Ajouter la première couleur détectée ou None si aucune couleur n'a été détectée
        detected_colors.append(colors_found[0] if colors_found else None)

    # Afficher un rectangle autour de chaque ROI
    for roi in rois:
        cv2.rectangle(image, (roi[0], roi[1]), (roi[2], roi[3]), (0, 255, 0), 2)

    # Afficher l'image avec les ROIs
    cv2.imshow(WINDOW_NAME, image)

    return detected_colors

def cmd_receive(cmd, game):
    if cmd == "reset":
        game.reset_game()
    
def color_name_to_color_code(valeur):
    if valeur == 'vert_clair':
        return np.array([50, 50, 150]), np.array([70, 255, 255]) # Vert clair
    # elif valeur == 'vert_fonce':
    #     return np.array([75, 130, 100]), np.array([85, 180, 150])  # Vert foncé
    elif valeur == 'bleu_clair':
        return np.array([96, 60, 150]), np.array([112, 110, 255])  # Bleu clair
    elif valeur == 'violet':
        return np.array([110, 80, 80]), np.array([130, 210, 255])  # Violet
    elif valeur == 'rouge':
        return np.array([0, 150, 50]), np.array([10, 255, 255])  # Rouge
    elif valeur == 'rose':
        return np.array([130, 50, 50]), np.array([170, 255, 255])  # Rose
    elif valeur == 'jaune':
         return np.array([25, 140, 150]), np.array([35, 255, 255])  # Jaune
    else:
        print('Couleur mal orthographiée ou non prise en compte')
        return None, None



if __name__ == '__main__':
    all_colors = ['vert_clair', 'bleu_clair', 'violet', 'rouge', 'rose', 'jaune']

    # Crée une instance du jeu
    game = MastermindGame(all_colors)
    mqtt_service = start_mqtt_service(on_cmd_receive=lambda cmd: cmd_receive(cmd, game))

    # Capture du flux vidéo (0 pour /dev/video0)
    capture = cv2.VideoCapture(0, cv2.CAP_V4L2)  # Vous pouvez essayer cv2.CAP_V4L2 pour forcer V4L2

    if not capture.isOpened():
        print("Erreur : Impossible d'accéder à la caméra.")
        exit()

    # Définir 4 zones d'intérêt (ROI) au format (x1, y1, x2, y2)
    rois = [
        (30, 200, 130, 300),  # ROI 1
        (190, 200, 290, 300),  # ROI 2
        (350, 200, 450, 300),  # ROI 3
        (510, 200, 610, 300)   # ROI 4
    ]

    # Suivi du temps pour afficher les résultats toutes les 0,5 secondes
    start_time = time.time()

    while True:
        ret, image = capture.read()

        if not ret or image is None:
            print("Erreur : Impossible de lire le flux de la caméra.")
            continue

        detected_colors = track(image, game.color_ranges, rois)

        # Affiche les couleurs détectées toutes les 0,5 secondes
        current_time = time.time()
        if current_time - start_time >= 0.5:
            if all(color is not None for color in detected_colors):
                if game.backup_detected_colors != detected_colors:
                    print(f"Couleurs détectées : {detected_colors}")
                    game.backup_detected_colors = detected_colors
                    game.essais += 1
                    print(f"Essais : {game.essais} / {game.essais_max}")
                    game.update_results(detected_colors)
                    
                    # Envoyer les résultats MQTT
                    data = {'correcte': game.correcte, 'mal_place': game.malPlace, 'essais' : f"{game.essais}/{game.essais_max}"}
                    mqtt_service.publish(data, "/detection/mastermind/led")
                    
                    if game.correcte == 4:
                        print("bravo !!!!")
                        data = "finish"
                        mqtt_service.publish(data, "/detection/mastermind/status")
                    elif game.essais >= game.essais_max:
                        game.reset_game()

            start_time = current_time
            

        # Quitter avec la touche ESC
        if cv2.waitKey(1) & 0xFF == 27:
            break

    # Libérer les ressources
    capture.release()
    cv2.destroyAllWindows()
    mqtt_service.stop()

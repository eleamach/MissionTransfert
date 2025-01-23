import cv2
import numpy as np

def nothing(x):
    pass

# Crée une fenêtre avec des sliders pour ajuster les plages HSV
cv2.namedWindow("Trackbars")
cv2.createTrackbar("H Lower", "Trackbars", 0, 179, nothing)
cv2.createTrackbar("H Upper", "Trackbars", 179, 179, nothing)
cv2.createTrackbar("S Lower", "Trackbars", 0, 255, nothing)
cv2.createTrackbar("S Upper", "Trackbars", 255, 255, nothing)
cv2.createTrackbar("V Lower", "Trackbars", 0, 255, nothing)
cv2.createTrackbar("V Upper", "Trackbars", 255, 255, nothing)

# Capture vidéo
capture = cv2.VideoCapture(0)

if not capture.isOpened():
    print("Erreur : Impossible d'accéder à la caméra.")
    exit()

while True:
    ret, frame = capture.read()
    if not ret:
        print("Erreur : Impossible de lire le flux vidéo.")
        break

    # Convertir en HSV
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

    # Récupérer les valeurs des sliders
    h_lower = cv2.getTrackbarPos("H Lower", "Trackbars")
    h_upper = cv2.getTrackbarPos("H Upper", "Trackbars")
    s_lower = cv2.getTrackbarPos("S Lower", "Trackbars")
    s_upper = cv2.getTrackbarPos("S Upper", "Trackbars")
    v_lower = cv2.getTrackbarPos("V Lower", "Trackbars")
    v_upper = cv2.getTrackbarPos("V Upper", "Trackbars")

    # Appliquer le masque
    lower_bound = np.array([h_lower, s_lower, v_lower])
    upper_bound = np.array([h_upper, s_upper, v_upper])
    mask = cv2.inRange(hsv, lower_bound, upper_bound)

    # Afficher l'image originale et le masque
    cv2.imshow("Original", frame)
    cv2.imshow("Mask", mask)

    # Quitter avec la touche ESC
    if cv2.waitKey(1) & 0xFF == 27:
        break

capture.release()
cv2.destroyAllWindows()

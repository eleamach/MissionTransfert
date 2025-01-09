import time

class EmotionValidator:
    def __init__(self):
        # Mémorisation des états pour chaque niveau
        self.state = {
            1: [],
            2: [],
            3: []
        }
        # Horodatage pour chaque niveau
        self.start_time = {
            1: None,
            2: None,
            3: None
        }
        self.level = 1
        self.fini = False
    
    def reset(self):
        self.level = 1
        self.state = {
            1: [],
            2: [],
            3: []
        }
        # Horodatage pour chaque niveau
        self.start_time = {
            1: None,
            2: None,
            3: None
        }
        self.fini = False

    def validateLevel(self, detected_emotions):
        # Définitions des séquences à respecter pour chaque niveau
        sequences = {
            1: ['Sad', 'Happy', 'Natural'],
            2: [
                ['Happy', 'Sad'],  # Étape 1 : triste et heureux en même temps
                ['Surprise', 'Angry']  # Étape 2 : surprise et colère en même temps
            ],
            3: ['Sad', 'Angry', 'Disgust']
        }

        time_limits = {
            1: 40,  # 20 secondes pour le niveau 1
            2: None,  # Pas de limite de temps pour le niveau 2
            3: 30   # 10 secondes pour le niveau 3
        }

        if self.level not in sequences:
            if self.fini == False:
                self.fini = True
                return "Fin"  # Niveau invalide
            return "Attente"

        # Initialiser le temps de début si ce n'est pas encore fait
        if self.level != 2 and self.start_time[self.level] is None and detected_emotions:
            # Vérifier si la première émotion détectée est dans la séquence attendue
            if sequences[self.level][0] in detected_emotions[0]:
                self.start_time[self.level] = time.time()  # Initialiser le temps avec la première émotion valide

        # Vérifier le délai imparti pour les niveaux 1 et 3
        if self.level != 2 and self.start_time[self.level] is not None:
            elapsed_time = time.time() - self.start_time[self.level]
            if elapsed_time > time_limits[self.level]:
                self.state[self.level] = []  # Réinitialiser l'avancement du niveau
                self.start_time[self.level] = None  # Réinitialiser le temps pour permettre de recommencer
                return "Temps fini"

        if self.level == 1:
            # Niveau 1 : faire les émotions dans l'ordre
            expected_sequence = sequences[1]

            # Ajouter uniquement les émotions détectées qui font partie de la séquence attendue
            for emotion in detected_emotions:
                if emotion in expected_sequence:
                    if len(self.state[self.level]) < len(expected_sequence):
                        # Ajouter l'émotion si elle correspond à la prochaine attendue
                        if emotion == expected_sequence[len(self.state[self.level])]:
                            self.state[self.level].append(emotion)
            res = str(self.level) + "/" + str(self.state[self.level])
            if self.state[self.level] == expected_sequence:
                self.state[self.level] = []  # Réinitialiser une fois validé
                self.start_time[self.level] = None  # Réinitialiser le temps
                self.level += 1
            return res

        elif self.level == 2:
            # Niveau 2 : 2 étapes avec des émotions en simultané
            current_stage = len(self.state[self.level])
            temp = []

            if current_stage < len(sequences[2]):
                required_emotions = sequences[2][current_stage]
                detected = []

                # Filtrer les émotions détectées qui font partie des émotions requises
                for emotion in detected_emotions:
                    if emotion in required_emotions:
                        detected.append(emotion)

                # Si toutes les émotions requises sont détectées
                if all(emotion in detected_emotions for emotion in required_emotions):
                    self.state[self.level].append(required_emotions)
                    temp = self.state[self.level].copy()  # Copie pour ne pas affecter l'état
                else:
                    # Ajouter l'état actuel validé
                    if self.state[self.level]:
                        temp = self.state[self.level].copy()  # Copie de l'état validé actuel
                    # Ajouter une liste vide si aucune bonne émotion n'est détectée
                    if not detected:
                        temp.append([])
                    else:
                        temp.append(detected)

            res = str(self.level) + "/" + str(temp)
            if len(self.state[self.level]) == len(sequences[2]):
                self.state[self.level] = []  # Réinitialiser une fois validé
                self.start_time[self.level] = None  # Réinitialiser le temps
                self.level += 1
            return res


        elif self.level == 3:
            # Niveau 3 : faire les émotions dans l'ordre exact
            expected_sequence = sequences[3]

            # Ajouter uniquement les émotions détectées qui font partie de la séquence attendue
            for emotion in detected_emotions:
                if emotion in expected_sequence:
                    if len(self.state[self.level]) < len(expected_sequence):
                        # Ajouter l'émotion si elle correspond à la prochaine attendue
                        if emotion == expected_sequence[len(self.state[self.level])]:
                            self.state[self.level].append(emotion)
            res = str(self.level) + "/" + str(self.state[self.level])
            if self.state[self.level] == expected_sequence:
                self.state[self.level] = []  # Réinitialiser une fois validé
                self.start_time[self.level] = None  # Réinitialiser le temps
                self.level += 1
            return res

         # Si non validé


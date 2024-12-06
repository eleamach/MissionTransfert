# -*- coding: utf-8 -*-
import qi
import json
import logging
import time

class PepperService(object):
    APP_ID = "com.elea.pepperservice"

    def __init__(self, qiapp):
        self.qiapp = qiapp
        self.session = qiapp.session
        self.tts = self.session.service("ALTextToSpeech")
        self.tts.setLanguage("French")
        self.audio_player = self.session.service("ALAudioPlayer")
        self.logger = logging.getLogger(self.APP_ID)
        logging.basicConfig(level=logging.INFO)
        self.motion = self.session.service("ALMotion")
        self.posture = self.session.service("ALRobotPosture")
        self.animation_player = self.session.service("ALAnimationPlayer")
        self.tts.setParameter("speed", 110)
        self.tts.setParameter("pitchShift", 1.1)
        self.memory = self.session.service("ALMemory")
        self.active_subscriptions = set()  # Liste des abonnements actifs
        
        self.head_touch_subscription = self.memory.subscriber("TouchChanged")
        self.head_touch_subscription.signal.connect(self.on_touch_changed)
    
    @qi.bind(returnType=qi.Void, paramsType=[qi.String])
    def subscribe(self, speech_recognition, module_name):
        if module_name not in self.active_subscriptions:
            try:
                speech_recognition.subscribe(module_name)
                self.active_subscriptions.add(module_name)
                self.logger.info("Abonnement réussi à {}".format(module_name))
            except Exception as e:
                self.logger.error("Erreur lors de l'abonnement à {} : {}".format(module_name, e))
        else:
            self.logger.warning("{} est déjà abonné.".format(module_name))

    @qi.bind(returnType=qi.Void, paramsType=[qi.String])
    def unsubscribe(self, speech_recognition, module_name):
        """Désabonne un module s'il est abonné."""
        if module_name in self.active_subscriptions:
            try:
                speech_recognition.unsubscribe(module_name)
                self.active_subscriptions.remove(module_name)
                self.logger.info("Désabonnement réussi de {}".format(module_name))
            except Exception as e:
                self.logger.error("Erreur lors du désabonnement de {} : {}".format(module_name, e))
        else:
                self.logger.warning("Tentative de désabonnement de {}, mais il n'était pas abonné.".format(module_name))

    
    @qi.bind(returnType=qi.Void, paramsType=[qi.String])
    def on_touch_changed(self, data):
        try:
            for touch in data:
                sensor_name = touch[0]
                is_touched = touch[1]
                if sensor_name in ["Head/Touch/Front", "Head/Touch/Middle", "Head/Touch/Rear"] and is_touched:
                    self.indice()
                    break
        except Exception as e:
            self.logger.error("Erreur lors du traitement des données tactiles : {}".format(e))

    @qi.bind(returnType=qi.Void, paramsType=[])
    def indice(self):
        try:
            self.tts.say("Un indice peut-être ?")
            speech_recognition = self.session.service("ALSpeechRecognition")
            vocabulary = ["oui", "non"]
            speech_recognition.pause(True)
            self.logger.info("Définition du vocabulaire : {}".format(vocabulary))
            speech_recognition.setVocabulary(vocabulary, False)
            speech_recognition.pause(False)

            memory = self.session.service("ALMemory")
            word_recognized_subscriber = memory.subscriber("WordRecognized")
            word_recognized_subscriber.signal.connect(self.on_word_recognized)
            self.logger.info("Signal connecté à on_word_recognized.")
            self.subscribe(speech_recognition, "IndiceDialog")
            
            time.sleep(5)  # Donne du temps à l'utilisateur pour répondre
        except Exception as e:
            self.logger.error("Erreur lors de l'indice : {}".format(e))

    @qi.bind(returnType=qi.Void, paramsType=[qi.String])
    def on_word_recognized(self, data):
        try:
            word = data[0]
            confidence = data[1]            
            if confidence > 0.4:
                if word == "oui":
                    self.tts.say("D'accord, donnez-moi le nom de l'atelier.")
                    self.ask_for_workshop()
                elif word == "non":
                    self.tts.say("Très bien, pas d'indice.")
        except Exception as e:
            self.logger.error("Erreur lors du traitement de la reconnaissance vocale : {}".format(e))

    @qi.bind(returnType=qi.Void, paramsType=[])
    def ask_for_workshop(self):
        try:
            self.tts.say("Quel est le nom de l'atelier ?")
            speech_recognition = self.session.service("ALSpeechRecognition")
            speech_recognition.pause(True)
            workshops = ["Détéction", "Capteur", "IA"] 
            speech_recognition.setVocabulary(workshops, False)
            speech_recognition.pause(False)
            workshop_recognition_subscriber = self.memory.subscriber("WordRecognized")
            workshop_recognition_subscriber.signal.connect(self.on_workshop_recognized)
            self.subscribe(speech_recognition, "WorkshopDialog")
            time.sleep(5)
        except Exception as e:
            self.logger.error("Erreur lors de la demande du nom de l'atelier : {}".format(e))

    @qi.bind(returnType=qi.Void, paramsType=[qi.String])
    def on_workshop_recognized(self, data):
        try:
            print("debug1")
            workshop = data[0]  # Le nom de l'atelier reconnu
            confidence = data[1]  # La confiance de la reconnaissance vocale
            speech_recognition = self.session.service("ALSpeechRecognition")
            
            if confidence > 0.4:
                self.tts.say("Vous avez choisi l'atelier {}. Voulez-vous le premier indice, le second indice ou la solution ?".format(workshop))
                self.selected_workshop = workshop  # Sauvegarder l'atelier sélectionné
                self.ask_for_hint()  # Demander l'indice
            self.unsubscribe(speech_recognition, "WorkshopDialog")
            
        except Exception as e:
            self.logger.error("Erreur lors de la reconnaissance du nom de l'atelier : {}".format(e))

    @qi.bind(returnType=qi.Void, paramsType=[])
    def ask_for_hint(self):
        try:
            self.tts.say("Voulez-vous le premier indice, le second indice, ou la solution ?")
            speech_recognition = self.session.service("ALSpeechRecognition")
            options = ["premier", "second", "solution"]  # Choix d'indice
            speech_recognition.pause(True)
            speech_recognition.setVocabulary(options, False)
            speech_recognition.pause(False)
            
            # Abonnement pour écouter le choix de l'utilisateur concernant l'indice
            memory = self.session.service("ALMemory")
            hint_recognition_subscriber = memory.subscriber("WordRecognized")
            hint_recognition_subscriber.signal.connect(self.on_hint_recognized)
            self.subscribe(speech_recognition, "HintDialog")
            
        except Exception as e:
            self.logger.error("Erreur lors de la demande du choix de l'indice : {}".format(e))


    @qi.bind(returnType=qi.Void, paramsType=[qi.String])
    def on_hint_recognized(self, data):
        try:
            choice = data[0]  # L'indice choisi
            confidence = data[1]  # La confiance de la reconnaissance vocale
            speech_recognition = self.session.service("ALSpeechRecognition")
            
            if confidence > 0.4:
                data = self.get_json("/path/to/your/json/file.json")  # Remplacez par le bon chemin
                if choice == "premier indice":
                    self.tts.say(data[self.selected_workshop][0])  # Affiche le premier indice
                elif choice == "second indice":
                    self.tts.say(data[self.selected_workshop][1])  # Affiche le second indice
                elif choice == "solution":
                    self.tts.say(data[self.selected_workshop][2])  # Affiche la solution
            
            # Désabonnement après la reconnaissance du choix
            self.unsubscribe(speech_recognition, "HintDialog")
            self.tts.say("Vous avez choisi {}. Merci de votre participation.".format(choice))  # Correction ici

        except Exception as e:
            self.logger.error("Erreur lors de la reconnaissance du choix de l'indice : {}".format(e))




    @qi.bind(returnType=qi.Void, paramsType=[qi.String])
    def get_json(self, file):
        try:
            with open(file, "r") as f:
                data = json.load(f)
                return data
        except Exception as e:
            self.logger.error("Erreur lors de la lecture du fichier JSON : {}".format(e))
            return {}

    @qi.bind(returnType=qi.Void, paramsType=[])
    def stop(self):
        try:
            self.cleanup_subscriptions()
            self.qiapp.stop()
        except Exception as e:
            self.logger.error("Erreur lors de l'arrêt de l'application : {}".format(e))

    def cleanup_subscriptions(self):
        """Désabonne tous les modules actifs."""
        speech_recognition = self.session.service("ALSpeechRecognition")
        for subscription in list(self.active_subscriptions):
            try:
                speech_recognition.unsubscribe(subscription)
                self.logger.info("Désabonnement réussi de {}".format(subscription))
            except Exception as e:
                self.logger.error("Erreur lors du désabonnement de {}".format(subscription))
        self.active_subscriptions.clear()

if __name__ == "__main__":
    import stk.runner
    stk.runner.run_service(PepperService)

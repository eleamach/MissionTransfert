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
        self.active_subscriptions = set()
        self.head_touch_subscription = self.memory.subscriber("TouchChanged")
        self.head_touch_subscription.signal.connect(self.on_touch_changed)
        self.tablet = self.session.service("ALTabletService")
        self.tablet.loadApplication("elea")
        self.tablet.showWebview()

        # Gestion des états
        self.current_state = "idle"  # État initial
        self.selected_workshop = None  # Atelier choisi
        self.hint = None  # Indice choisi

    def play_animation(self, animation_name):
        try:
            self.animation_player.run(animation_name) 
        except Exception as e:
            self.logger.error("Erreur lors du lancement de l'animation : {}".format(e))

    def raise_event(self, event_name, value):
        try:
            print("Raising event {} with value {}".format(event_name, value))
            self.memory.raiseEvent(event_name, value)
        except Exception as e:
            self.logger.error("Erreur lors de la levée de l'événement {}: {}".format(event_name, e))

    def speak_n_animate(self, id, datakey, animation, professors, maitre):
        try:
            if professors:
                self.tts.setParameter("speed", 110)
                self.tts.setParameter("pitchShift", 0.9)
            elif maitre:
                self.tts.setParameter("speed", 90)
                self.tts.setParameter("pitchShift", 0.8)
            else:
                self.tts.setParameter("speed", 110)
                self.tts.setParameter("pitchShift", 1.1)
            data = self.get_json("/home/elea.machillot/Documents/S1_G1_Machillot_Perbet_Jeannin_Delcamp/Pepper/Script/data.json")
            self.play_animation(animation) 
            self.tts.say(data[datakey][id])
        except Exception as e:
            self.logger.error("Erreur lors de la parole ou de l'animation: {}".format(e))

    def speak(self, arg):
        try:
            self.tts.say(arg)
        except Exception as e:
            self.logger.error("Erreur lors de la parole : {}".format(e))

    def play_music(self, file):        
        try:
            file_id = self.audio_player.loadFile(file)
            self.audio_player.play(file_id)
            self.running_audio_ids = [file_id]
        except Exception as e:
            self.logger.error("Erreur lors de la lecture de la musique : {}".format(e))

    def subscribe(self, speech_recognition, module_name):
        if module_name not in self.active_subscriptions:
            try:
                speech_recognition.subscribe(module_name)
                self.active_subscriptions.add(module_name)
            except Exception as e:
                self.logger.error("Erreur lors de l'abonnement à {} : {}".format(module_name, e))
        else:
            self.logger.warning("{} est déjà abonné.".format(module_name))

    def unsubscribe(self, speech_recognition, module_name):
        if module_name in self.active_subscriptions:
            try:
                speech_recognition.unsubscribe(module_name)
                self.active_subscriptions.remove(module_name)
            except Exception as e:
                self.logger.error("Erreur lors du désabonnement de {} : {}".format(module_name, e))
        else:
                self.logger.warning("Tentative de désabonnement de {}, mais il n'était pas abonné.".format(module_name))

    def on_touch_changed(self, data):
        try:
            for touch in data:
                sensor_name = touch[0]
                is_touched = touch[1]
                if sensor_name in ["Head/Touch/Front", "Head/Touch/Middle", "Head/Touch/Rear"] and is_touched:
                    self.tts.say("Un indice peut-être ?")
                    speech_recognition = self.session.service("ALSpeechRecognition")
                    vocabulary = ["oui", "non"]
                    speech_recognition.pause(True)
                    speech_recognition.setVocabulary(vocabulary, False)
                    speech_recognition.pause(False)
                    memory = self.session.service("ALMemory")
                    word_recognized_subscriber = memory.subscriber("WordRecognized")
                    word_recognized_subscriber.signal.connect(self.on_word_recognized)
                    self.subscribe(speech_recognition, "IndiceDialog")
                    self.current_state = "choix_atelier"  # Mise à jour de l'état
                    time.sleep(10)
                    break
        except Exception as e:
            self.logger.error("Erreur lors du traitement des données tactiles : {}".format(e))

    def on_word_recognized(self, data):
        try:
            word = data[0]
            confidence = data[1]            
            if confidence > 0.5:
                if self.current_state == "choix_atelier":
                    if word == "oui":
                        self.tts.say("A quel atelier êtes-vous ? Si vous avez un doute sur le nom, regardez l'étiquette vers votre atelier") 
                        speech_recognition = self.session.service("ALSpeechRecognition")
                        speech_recognition.pause(True)
                        workshops = ["Simon", "Mastermind", "Texte", "Tof", "Bouton", "Onsaitpas", "Visage", "Camera", "Emotion", "Reconnaissance", "Images", "Labyrinthe"] 
                        speech_recognition.setVocabulary(workshops, False)
                        speech_recognition.pause(False)
                        workshop_recognition_subscriber = self.memory.subscriber("WordRecognized")
                        workshop_recognition_subscriber.signal.connect(self.on_workshop_recognized)
                        self.subscribe(speech_recognition, "WorkshopDialog")
                        self.current_state = "attente_atelier"  # Mise à jour de l'état
                        time.sleep(10)
                    elif word == "non":
                        self.tts.say("Très bien, pas d'indice.")
                        self.current_state = "idle"  # Retour à l'état initial
        except Exception as e:
            self.logger.error("Erreur lors du traitement de la reconnaissance vocale : {}".format(e))

    def on_workshop_recognized(self, data):
        try:
            workshop = data[0]  
            confidence = data[1] 
            if self.current_state == "attente_atelier" and confidence > 0.5:
                self.tts.say("Vous avez choisi l'atelier {}. Voulez-vous le premier indice, le deuxième ou le troisième ?".format(workshop))
                self.selected_workshop = workshop
                speech_recognition = self.session.service("ALSpeechRecognition")
                speech_recognition.pause(True)
                options = ["premier", "deuxième", "troisième"]  
                speech_recognition.setVocabulary(options, False)
                speech_recognition.pause(False)
                hint_recognition_subscriber = self.memory.subscriber("WordRecognized")
                hint_recognition_subscriber.signal.connect(self.on_hint_recognized)
                self.subscribe(speech_recognition, "HintDialog")
                self.current_state = "attente_indice"  # Mise à jour de l'état
                time.sleep(10)
        except Exception as e:
            self.logger.error("Erreur lors de la reconnaissance du nom de l'atelier : {}".format(e))

    def on_hint_recognized(self, data):
        try:
            word = data[0]
            confidence = data[1]
            if self.current_state == "attente_indice" and confidence > 0.5:
                if word == "premier":
                    self.hint = "0"
                elif word == "deuxième":
                    self.hint = "1"
                elif word == "troisième":
                    self.hint = "2" 

                if self.hint in ["0", "1", "2"]: 
                    json_data = self.get_json("/home/elea.machillot/Documents/S1_G1_Machillot_Perbet_Jeannin_Delcamp/Pepper/Script/indice.json")
                    self.tts.say(json_data[self.selected_workshop][int(self.hint)])
                else:
                    self.tts.say("Je n'ai pas compris votre choix.")
                self.current_state = "idle"  # Retour à l'état initial
        except Exception as e:
            self.logger.error("Erreur lors de la reconnaissance du choix de l'indice : {}".format(e))

    def get_json(self, file):
        try:
            with open(file, "r") as f:
                data = json.load(f)
                return data
        except Exception as e:
            self.logger.error("Erreur lors de la lecture du fichier JSON : {}".format(e))
            return {}

    def stop(self):
        try:
            self.cleanup_subscriptions()
            self.qiapp.stop()
        except Exception as e:
            self.logger.error("Erreur lors de l'arrêt de l'application : {}".format(e))

    def cleanup_subscriptions(self):
        speech_recognition = self.session.service("ALSpeechRecognition")
        for subscription in list(self.active_subscriptions):
            try:
                speech_recognition.unsubscribe(subscription)
            except Exception as e:
                self.logger.error("Erreur lors du désabonnement de {}".format(subscription))
        self.active_subscriptions.clear()

if __name__ == "__main__":
    import stk.runner
    stk.runner.run_service(PepperService)

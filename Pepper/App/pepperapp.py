# -*- coding: utf-8 -*-
import qi

from time import sleep

IP="10.50.90.104"
PORT=9559

def main():
    try:
        connection_url = "tcp://{}:{}".format(IP, PORT)
        app = qi.Application(["PepperRun", "--qi-url={}".format(connection_url)])
        app.start()
        session = app.session

        pepper_service = session.service("PepperService")
        try:
            # Page d'accueil
            pepper_service.raise_event("SimpleWeb/Page/Start", 1)
            sleep(10)
            partie_1(pepper_service)
            sleep(1)
            partie_2(pepper_service)
            sleep(1)
            partie_3(pepper_service)
            sleep(1)
            partie_4(pepper_service)
            sleep(1)
            partie_5(pepper_service)
            sleep(1)
            partie_6(pepper_service)
            
            
        except Exception as e:
            print("Error: {}".format(e))

    except RuntimeError as e:
        print("Error: {}".format(e))


def partie_1(pepper_service):
    """pepper_service.speak_n_animate(0, "intro","animations/Stand/BodyTalk/Listening/Listening_1", False, False)
    pepper_service.speak_n_animate(1, "intro", "animations/Stand/Gestures/Hey_6", False, False)
    pepper_service.speak_n_animate(2, "intro", "animations/Stand/BodyTalk/Listening/Listening_1", False, False)
    pepper_service.speak_n_animate(3, "intro", "animations/Stand/Reactions/TouchHead_4", False, False)
    pepper_service.speak_n_animate(6, "intro", "animations/Stand/Emotions/Positive/Laugh_2", False, False) 
    pepper_service.speak_n_animate(4, "intro","animations/Stand/BodyTalk/Listening/Listening_1", False, False) 
    pepper_service.speak_n_animate(5, "intro","animations/Stand/BodyTalk/Listening/Listening_1", False, False) """
    pepper_service.raise_event("SimpleWeb/Page/Progress0", 1)
    pepper_service.play_music("/home/nao/.local/share/PackageManager/apps/elea/clock.mp3")

def partie_2(pepper_service):
    pepper_service.speak_n_animate(0, "premierlot","animations/Stand/Waiting/Robot_1", False, False)
    pepper_service.raise_event("SimpleWeb/Page/Progress33", 1)
    pepper_service.speak_n_animate(1, "premierlot","animations/Stand/Emotions/Positive/Laugh_2", False, False)
    pepper_service.speak_n_animate(2, "premierlot","animations/Stand/Emotions/Negative/Fear_1", True, False)
    pepper_service.speak_n_animate(3, "premierlot","animations/Stand/Emotions/Negative/Fear_1", False, False)
    pepper_service.play_music("/home/nao/.local/share/PackageManager/apps/elea/clock.mp3")

def partie_3(pepper_service):
    pepper_service.speak_n_animate(0, "deuxiemelot","animations/Stand/Waiting/Robot_1", False, False)
    pepper_service.raise_event("SimpleWeb/Page/Progress66", 1)
    pepper_service.speak_n_animate(1, "deuxiemelot","animations/Stand/Emotions/Negative/Fear_1", True, False)
    pepper_service.speak_n_animate(2, "deuxiemelot","animations/Stand/Emotions/Negative/Fear_1", False, False)
    pepper_service.play_music("/home/nao/.local/share/PackageManager/apps/elea/clock.mp3")

def partie_4(pepper_service):
    pepper_service.speak_n_animate(0, "deuxiemelot","animations/Stand/Waiting/Robot_1", False, False)
    pepper_service.raise_event("SimpleWeb/Page/Progress99", 1)
    pepper_service.speak_n_animate(0, "code","animations/Stand/BodyTalk/Listening/Listening_1", False, False)
    pepper_service.speak_n_animate(1, "code","animations/Stand/Emotions/Positive/Confident_1", False, False)
    pepper_service.speak_n_animate(2, "code","animations/Stand/Gestures/YouKnowWhat_1", False, False)
    pepper_service.play_music("/home/nao/.local/share/PackageManager/apps/elea/clock.mp3")

def partie_5(pepper_service):
    pepper_service.speak_n_animate(0, "finalreussi","animations/Stand/Waiting/Robot_1", False, False)
    pepper_service.raise_event("SimpleWeb/Page/Reussi", 1)
    pepper_service.speak_n_animate(1, "finalreussi","animations/Stand/Emotions/Negative/Fear_1", True, False)
    pepper_service.speak_n_animate(2, "finalreussi","animations/Stand/Emotions/Negative/Fear_1", False, False)
    pepper_service.speak_n_animate(3, "finalreussi","animations/Stand/Emotions/Negative/Fear_1", True, False)
    pepper_service.speak_n_animate(4, "finalreussi","animations/Stand/Emotions/Negative/Fear_1", False, False)
    pepper_service.speak_n_animate(5, "finalreussi","animations/Stand/Gestures/Kisses_1", False, False)

def partie_6(pepper_service):
    pepper_service.speak_n_animate(0, "finalreussi","animations/Stand/Waiting/Robot_1", False, False)
    pepper_service.raise_event("SimpleWeb/Page/Perdu", 1)
    pepper_service.speak_n_animate(0, "finalperdu","animations/Stand/BodyTalk/Listening/Listening_1", False, False)
    pepper_service.speak_n_animate(1, "finalperdu","animations/Stand/BodyTalk/Listening/Listening_1", False, False)
    pepper_service.speak_n_animate(2, "finalperdu","animations/Stand/BodyTalk/Listening/Listening_1", False, False)
    pepper_service.speak_n_animate(3, "finalperdu","animations/Stand/BodyTalk/Listening/Listening_1", False, False)
    pepper_service.speak_n_animate(4, "finalperdu","animations/Stand/BodyTalk/Listening/Listening_1", False, True)



if __name__ == "__main__":
    main()

from flask import Flask, render_template, request, jsonify, session
import sounddevice as sd
import soundfile as sf
import numpy as np
import os
from openai import OpenAI
from dotenv import load_dotenv
import paho.mqtt.client as mqtt
from flask_socketio import SocketIO

# Chargement des variables d'environnement
load_dotenv()

app = Flask(__name__)
app.secret_key = 'ia_vocal'  # Nécessaire pour les sessions
socketio = SocketIO(app)

# Configuration de l'enregistrement
SAMPLE_RATE = 44100
CHANNELS = 1

# Configuration OpenAI
client = OpenAI(api_key=os.getenv('OPENAI_API_KEY'))

# Énigmes et leurs réponses
ENIGMES = {
    1: {
        "question": "Je suis grand comme un arbre, mais je ne suis pas un arbre. Je parle sans bouche et marche sans pieds. Qui suis-je ?",
        "reponse": "une ombre"
    },
    2: {
        "question": "Plus j'ai de gardiens, moins je suis en sécurité. Qui suis-je ?",
        "reponse": "un secret"
    },
    3: {
        "question": "Je commence la nuit et je termine le matin. Qui suis-je ?",
        "reponse": "la lettre n"
    },
    4: {
        "question": "Je grandis sans être vivant, je n'ai pas de poumons mais j'ai besoin d'air, je n'ai pas de bouche mais je peux tuer. Qui suis-je ?",
        "reponse": "le feu"
    },
    5: {
        "question": "Les riches en manquent, les pauvres en ont, si on en mange on meurt. Qui suis-je ?",
        "reponse": "rien"
    }
}

ENIGME_FINALE = {
    "question": """Félicitations !
    
1. Je suis née dans un jardin, 
   Les poètes m'ont chanté mille fois,
   Symbole d'amour et de passion,
   Mais attention à mes épines qui peuvent te piquer les doigts.

2. Dans le carré magique de Saturne,
   Retourné je reste le même,
   Et les chats me considèrent comme leur nombre de vies.
""",
    "reponse": "rose 9"
}

# Configuration MQTT
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    client.subscribe("/ia/vocale/status")

def on_message(client, userdata, msg):
    if msg.payload.decode() == "reset":
        # Envoyer un signal au client via WebSocket
        socketio.emit('reset_app')

# Setup MQTT client
mqtt_client = mqtt.Client()
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message

# Connexion au broker MQTT
try:
    mqtt_client.connect("134.214.51.148", 1883, 60)
    mqtt_client.loop_start()
except Exception as e:
    print(f"Erreur de connexion MQTT: {e}")

@app.route('/')
def index():
    session['current_enigme'] = 1
    session['enigmes_resolues'] = 0
    session['phase'] = 'enigmes'  # nouvelle phase: 'enigmes' ou 'finale'
    return render_template('index.html', enigme=ENIGMES[1]['question'], phase='enigmes')

@app.route('/get_enigme', methods=['POST'])
def get_enigme():
    try:
        current_enigme = session.get('current_enigme', 1)
        
        # Demander à GPT-4 d'énoncer l'énigme de manière engageante
        response = client.chat.completions.create(
            model="gpt-4",
            messages=[
                {"role": "system", "content": "Tu es un maître des énigmes mystérieux et captivant. Énonce l'énigme de manière théâtrale et intrigante."},
                {"role": "user", "content": f"Énonce cette énigme de manière captivante : {ENIGMES[current_enigme]['question']}"}
            ]
        )
        
        # Convertir le texte en parole avec l'API TTS
        speech = client.audio.speech.create(
            model="tts-1",
            voice="nova",
            input=response.choices[0].message.content
        )
        
        # Sauvegarder l'audio
        audio_file = f'recordings/enigme_{current_enigme}.mp3'
        speech.stream_to_file(audio_file)
        
        return jsonify({
            'success': True,
            'audio_file': audio_file,
            'enigme_text': ENIGMES[current_enigme]['question']
        })
        
    except Exception as e:
        return jsonify({'success': False, 'message': str(e)})

@app.route('/record', methods=['POST'])
def record():
    try:
        if 'audio' not in request.files:
            return jsonify({'success': False, 'message': 'Aucun fichier audio reçu'})
            
        audio_file = request.files['audio']
        filename = 'recordings/current_recording.wav'
        os.makedirs('recordings', exist_ok=True)
        audio_file.save(filename)
        
        with open(filename, "rb") as audio_file:
            transcript = client.audio.transcriptions.create(
                model="whisper-1",
                file=audio_file
            )

        current_enigme = session.get('current_enigme', 1)
        
        # Si on est à l'énigme finale
        if session.get('phase') == 'finale':
            response = client.chat.completions.create(
                model="gpt-4",
                messages=[
                    {"role": "system", "content": f"Tu es un juge d'énigmes. La réponse attendue est 'rose 9'. Compare la réponse donnée. Réponds uniquement par 'CORRECT' ou 'INCORRECT'."},
                    {"role": "user", "content": transcript.text}
                ]
            )
            is_correct = response.choices[0].message.content == "CORRECT"
            return jsonify({
                'success': True,
                'message': "Félicitations, vous avez résolu l'énigme finale!" if is_correct else "Ce n'est pas la bonne réponse, essayez encore.",
                'transcript': transcript.text,
                'is_correct': is_correct,
                'phase': 'finale'
            })
            
        # Pour les énigmes normales
        response = client.chat.completions.create(
            model="gpt-4",
            messages=[
                {"role": "system", "content": f"Tu es un juge d'énigmes. La réponse attendue est '{ENIGMES[current_enigme]['reponse']}'. Compare la réponse donnée. Réponds uniquement par 'CORRECT' ou 'INCORRECT'."},
                {"role": "user", "content": transcript.text}
            ]
        )
        
        is_correct = response.choices[0].message.content == "CORRECT"
        
        if is_correct:
            session['enigmes_resolues'] = session.get('enigmes_resolues', 0) + 1
            session['current_enigme'] = current_enigme + 1
            
            # Si on vient de résoudre la 5ème énigme
            if session['enigmes_resolues'] == 5:
                session['phase'] = 'finale'
                return jsonify({
                    'success': True,
                    'message': "Bravo ! Vous avez résolu toutes les énigmes !",
                    'transcript': transcript.text,
                    'is_correct': True,
                    'enigmes_resolues': 5,
                    'phase': 'finale',
                    'final_enigme': ENIGME_FINALE['question']
                })
            else:
                next_enigme = ENIGMES[session['current_enigme']]['question']
                return jsonify({
                    'success': True,
                    'message': "Correct !",
                    'transcript': transcript.text,
                    'is_correct': True,
                    'enigmes_resolues': session['enigmes_resolues'],
                    'next_enigme': next_enigme
                })
        else:
            return jsonify({
                'success': True,
                'message': "Incorrect, essayez encore.",
                'transcript': transcript.text,
                'is_correct': False,
                'enigmes_resolues': session['enigmes_resolues']
            })
    
    except Exception as e:
        return jsonify({'success': False, 'message': str(e)})

@app.route('/regenerate_enigme', methods=['POST'])
def regenerate_enigme():
    try:
        current_enigme = session.get('current_enigme', 1)
        
        # Demander à GPT-4 de générer une nouvelle énigme
        response = client.chat.completions.create(
            model="gpt-4",
            messages=[
                {"role": "system", "content": f"""Génère une énigme différente mais dont la réponse doit être exactement : 
                '{ENIGMES[current_enigme]['reponse']}'.
                Renvoie uniquement l'énigme, sans la réponse."""},
                {"role": "user", "content": "Génère une nouvelle énigme"}
            ]
        )
        
        nouvelle_enigme = response.choices[0].message.content
        ENIGMES[current_enigme]['question'] = nouvelle_enigme
        
        return jsonify({
            'success': True,
            'nouvelle_enigme': nouvelle_enigme
        })
        
    except Exception as e:
        return jsonify({'success': False, 'message': str(e)})

if __name__ == '__main__':
    socketio.run(app, debug=True)

from flask import Flask, render_template, jsonify, request
from openai import OpenAI
from dotenv import load_dotenv
import os
import threading

# Charger les variables d'environnement
load_dotenv()

app = Flask(__name__)
client = OpenAI(api_key=os.getenv('OPENAI_API_KEY'))

# Variable globale pour stocker le contenu pré-généré
cached_content = {
    'text': None,
    'audio_path': None
}

CORRECT_COUNT = 5  # Le nombre correct de "robotique"

def generate_text_with_keywords(keyword, occurrences):
    prompt = f"""Génère un texte cohérent en français d'environ 100 mots qui utilise 
    le mot '{keyword}' exactement {occurrences} fois. Le texte doit rester naturel 
    et avoir du sens."""

    completion = client.chat.completions.create(
        model="gpt-3.5-turbo",
        messages=[{"role": "user", "content": prompt}]
    )
    
    return completion.choices[0].message.content

def convert_text_to_speech(text):
    output_file = "static/output_next.mp3"
    response = client.audio.speech.create(
        model="tts-1",
        voice="alloy",
        input=text
    )
    
    with open(output_file, 'wb') as file:
        for chunk in response.iter_bytes():
            file.write(chunk)
            
    return output_file

def generate_next_content():
    """Génère le prochain contenu en arrière-plan"""
    try:
        text = generate_text_with_keywords("robotique", 5)
        audio_path = convert_text_to_speech(text)
        
        global cached_content
        cached_content = {
            'text': text,
            'audio_path': '/static/output_next.mp3'
        }
    except Exception as e:
        print(f"Erreur dans la génération en arrière-plan: {e}")

# Générer le premier contenu au démarrage
generate_next_content()

@app.route('/')
def home():
    return render_template('index.html')

@app.route('/generate', methods=['POST'])
def generate():
    try:
        # Récupérer le contenu pré-généré
        current_content = cached_content.copy()
        
        # Renommer le fichier audio pour le contenu actuel
        if os.path.exists('static/output_next.mp3'):
            os.rename('static/output_next.mp3', 'static/output_current.mp3')
            current_content['audio_path'] = '/static/output_current.mp3'
        
        # Lancer la génération du prochain contenu en arrière-plan
        threading.Thread(target=generate_next_content).start()
        
        return jsonify({
            'success': True,
            'text': current_content['text'],
            'audio_path': current_content['audio_path']
        })
    except Exception as e:
        return jsonify({
            'success': False,
            'error': str(e)
        })

@app.route('/validate', methods=['POST'])
def validate():
    data = request.get_json()
    count = data.get('count', 0)
    
    if count == CORRECT_COUNT:
        print("REUSSI")
        return jsonify({'success': True})
    else:
        print("ECHEC")
        return jsonify({'success': False})

if __name__ == '__main__':
    app.run(debug=True)

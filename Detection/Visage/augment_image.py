from tensorflow.keras.preprocessing.image import ImageDataGenerator
import os
import numpy as np
from PIL import Image

# Définir les chemins de base
input_base_dir = "data/input"
output_base_dir = "data/augmented"

# Sous-dossiers
categories = ["known_person", "unknown_person", "objects"]

# Définir les augmentations
datagen = ImageDataGenerator(
    rotation_range=20,
    width_shift_range=0.2,
    height_shift_range=0.2,
    zoom_range=0.2,
    brightness_range=[0.8, 1.2],
    horizontal_flip=True,
    fill_mode='nearest'
)

# Parcourir chaque catégorie et appliquer les augmentations
for category in categories:
    input_dir = os.path.join(input_base_dir, category)
    output_dir = os.path.join(output_base_dir, category)
    os.makedirs(output_dir, exist_ok=True)  # Crée le dossier si nécessaire

    print(f"Traitement des images dans '{category}'...")

    for img_name in os.listdir(input_dir):
        if not img_name.lower().endswith(('.png', '.jpg', '.jpeg')):
            continue

        img_path = os.path.join(input_dir, img_name)
        try:
            img = Image.open(img_path)
            img = img.convert("RGB")  # Assurer un format compatible
            img_array = np.array(img)

            # Reshape pour le générateur
            img_array = img_array.reshape((1,) + img_array.shape)

            # Générer des augmentations
            i = 0
            for batch in datagen.flow(img_array, batch_size=1,
                                      save_to_dir=output_dir,
                                      save_prefix=f"{img_name.split('.')[0]}_aug",
                                      save_format='jpeg'):
                i += 1
                if i > 10:  # Générer 10 augmentations par image
                    break
        except Exception as e:
            print(f"Erreur avec l'image {img_name}: {e}")

print("Augmentations terminées.")
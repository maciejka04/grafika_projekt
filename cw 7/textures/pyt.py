from PIL import Image
import numpy as np

def generate_normal_map(input_path, output_path="normal_map.png", strength=2.0):
    # Wczytaj obraz i zamień na skale szarości
    image = Image.open(input_path).convert('L')
    pixels = np.asarray(image).astype('float32') / 255.0
    height, width = pixels.shape

    # Przygotuj pustą mapę normalną
    normal_map = np.zeros((height, width, 3), dtype='float32')

    # Oblicz gradienty
    for y in range(height):
        for x in range(width):
            sx = pixels[y, min(x + 1, width - 1)] - pixels[y, max(x - 1, 0)]
            sy = pixels[min(y + 1, height - 1), x] - pixels[max(y - 1, 0), x]

            normal = np.array([-sx * strength, -sy * strength, 1.0])
            normal /= np.linalg.norm(normal)

            # Przekształć do przestrzeni kolorów [0, 255]
            normal_map[y, x] = (normal * 0.5 + 0.5) * 255

    # Zapisz wynik
    normal_map_img = Image.fromarray(normal_map.astype('uint8'))
    normal_map_img.save(output_path)
    print(f"Zapisano: {output_path}")

# Przykładowe użycie
generate_normal_map("tower.png", "tower_normal.png", strength=4.0)

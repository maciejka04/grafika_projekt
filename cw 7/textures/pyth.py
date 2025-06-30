from PIL import Image
import numpy as np

def fast_normal_map(input_path, output_path="normal_map.png", strength=2.0):
    # Wczytaj i przekształć obraz do skali szarości
    img = Image.open(input_path).convert('L')
    height_map = np.array(img, dtype=np.float32) / 255.0

    # Oblicz gradienty
    dx = np.gradient(height_map, axis=1)
    dy = np.gradient(height_map, axis=0)

    # Znormalizuj wektory normalne
    dz = np.ones_like(dx) / strength
    normal = np.stack((-dx, -dy, dz), axis=-1)
    norm = np.linalg.norm(normal, axis=2, keepdims=True)
    normal /= norm

    # Przeskaluj do [0, 255]
    normal = (normal * 0.5 + 0.5) * 255.0
    normal = np.clip(normal, 0, 255).astype(np.uint8)

    # Zapisz jako obraz
    Image.fromarray(normal).save(output_path)
    print(f"Zapisano: {output_path}")

# Przykład użycia
fast_normal_map("tower.png", "tower_normal.png", strength=4.0)

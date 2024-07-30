from PIL import Image
import numpy as np

def get_image_matrix(image_path, width, height):
    img = Image.open(image_path)
    
    # Redimensionar a imagem
    img = img.resize((width, height))
    
    # Converte a imagem para RGB (caso não seja)
    img = img.convert('RGB')
    
    # Obtem os pixels da imagem como um array NumPy
    pixels = np.array(img)
    
    # Inicializa a matriz RGB com zeros
    quantized_pixels = np.zeros((height, width, 3), dtype=int)
    
    # Divide por 32 pra que fique em escala de 0 a 7
    for i in range(height):
        for j in range(width):
            r, g, b = pixels[i, j]  # pixels[i, j] já é uma tupla (r, g, b)
            quantized_pixels[i, j, 0] = r // 32
            quantized_pixels[i, j, 1] = g // 32
            quantized_pixels[i, j, 2] = b // 32
    
    return quantized_pixels

def save_matrix_to_file(matrix, file_path):
    with open(file_path, 'w') as file:
        for row in matrix:
            row_str = ','.join(f'{{{r},{g},{b}}}' for r, g, b in row)
            file.write(row_str + ',\n')

image_path = 'sprites/tela-game.png'
matrix = get_image_matrix(image_path, 80, 60)
save_matrix_to_file(matrix, 'matrix_rgb.txt')

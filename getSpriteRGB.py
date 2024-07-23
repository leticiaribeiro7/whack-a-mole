# from PIL import Image
# import numpy as np

# def get_image_matrix(image_path):
#     # Abrir a imagem
#     img = Image.open(image_path)
    
#     # Redimensionar a imagem para 20x20 pixels
#     img = img.resize((20, 20))
    
#     # Obter os pixels da imagem
#     pixels = np.array(img)
    
#     # Inicializar a matriz RGB com zeros
#     quantized_pixels = np.zeros((20, 20, 3), dtype=int)
    
#     # Quantizar cada valor RGB para a escala de 0 a 7
#     for i in range(20):
#         for j in range(20):
#             r, g, b = pixels[i, j][:3]  # Ignorar o canal alpha se houver
#             quantized_pixels[i, j, 0] = r // 32
#             quantized_pixels[i, j, 1] = g // 32
#             quantized_pixels[i, j, 2] = b // 32
    
#     return quantized_pixels

# def save_matrix_to_file(matrix, file_path):
#     # Salvar a matriz em um arquivo de texto
#     with open(file_path, 'w') as file:
#         for row in matrix:
#             row_str = ','.join(f'[{r},{g},{b}]' for r, g, b in row)
#             file.write(row_str + ',')

# # Exemplo de uso
# image_path = 'sprites/spark.png'
# matrix = get_image_matrix(image_path)
# save_matrix_to_file(matrix, 'matrix_rgb.txt')


from PIL import Image
import numpy as np

def get_image_matrix(image_path):
    # Abrir a imagem
    img = Image.open(image_path)
    
    # Redimensionar a imagem para 80x60 pixels
    img = img.resize((80, 60))
    
    # Obter os pixels da imagem
    pixels = np.array(img)
    
    # Inicializar a matriz RGB com zeros
    quantized_pixels = np.zeros((60, 80, 3), dtype=int)
    
    # Quantizar cada valor RGB para a escala de 0 a 7
    for i in range(60):
        for j in range(80):
            r, g, b = pixels[i, j][:3]  # Ignorar o canal alpha se houver
            quantized_pixels[i, j, 0] = r // 32
            quantized_pixels[i, j, 1] = g // 32
            quantized_pixels[i, j, 2] = b // 32
    
    return quantized_pixels

def save_matrix_to_file(matrix, file_path):
    # Salvar a matriz em um arquivo de texto
    with open(file_path, 'w') as file:
        for row in matrix:
            row_str = ','.join(f'[{r},{g},{b}]' for r, g, b in row)
            file.write(row_str + ',\n')

# Exemplo de uso
image_path = 'sprites/tela-inicial.png'
matrix = get_image_matrix(image_path)
save_matrix_to_file(matrix, 'matrix_rgb.txt')

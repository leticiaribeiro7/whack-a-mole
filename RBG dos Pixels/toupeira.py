from PIL import Image

# Abre a imagem
imagem = Image.open("imagens/toupeira.png")

# Redimensiona a imagem para 20x20, se necessário
imagem = imagem.resize((20, 20))

# Pega a largura e a altura da imagem
largura, altura = imagem.size

# Cria uma lista para armazenar os valores RGB divididos por 32 em formato de uma matriz
matriz_rgb = []

for y in range(altura):
    for x in range(largura):
        # Usa getpixel para obter o valor RGB do pixel
        cor = imagem.getpixel((x, y))

        # Divide os valores RGB por 32
        r, g, b = cor
        r_dividido = r // 32
        g_dividido = g // 32
        b_dividido = b // 32

        # Adiciona a cor dividida à lista
        matriz_rgb.append((r_dividido, g_dividido, b_dividido))

# Imprime a matriz de valores RGB divididos por 32
for cor in matriz_rgb:
    print(cor)


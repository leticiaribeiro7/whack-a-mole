from PIL import Image

# Abrir a imagem
imagem = Image.open("imagens/martelo pixel.jpg")

# Redimensionar a imagem para 20x20, se necessário
imagem = imagem.resize((20, 20))

# Pegar a largura e a altura da imagem
largura, altura = imagem.size

# Criar uma matriz para armazenar os valores RGB
matriz_rgb = []

for y in range(altura):
    linha = []
    for x in range(largura):
        # Usar getpixel para obter o valor RGB do pixel
        cor = imagem.getpixel((x, y))

        # Dividir os valores RGB por 32
        r, g, b = cor
        r_dividido = r // 32
        g_dividido = g // 32
        b_dividido = b // 32
        linha.append((r_dividido, g_dividido, b_dividido))
    matriz_rgb.append(linha)


# Imprimir a matriz de valores RGB
for linha in matriz_rgb:
    print(linha)

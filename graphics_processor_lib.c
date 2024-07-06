#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "graphics_processor.h"

#define WBR      1 /*Operação para escrever cor de fundo*/
#define WBM      2 /*Operação para escrever bloco de fundo*/
#define DP       3 /*Operação para definir polígono*/
#define WSM      4 /*Operação para escrever sprite na memória*/

#define MAX_SIZE 32

/**
 * \brief Define a cor de fundo usando valores RGB.
 * 
 * \param[in] R : Componente vermelho
 * \param[in] G : Componente verde
 * \param[in] B : Componente azul)
 * 
 * \return int : Número de bytes escritos ou -1 em caso de erro.
 */
void set_background_color(int R, int G, int B);


/**
 * \brief Define as propriedades de um sprite.
 * 
 * \param[in] reg : Registro do sprite
 * \param[in] x : Posição x do sprite
 * \param[in] y : Posição y do sprite
 * \param[in] offset : Deslocamento do sprite na memória
 * \param[in] activation_bit : Bit de ativação do sprite
 * 
 * \return Número de bytes escritos ou -1 em caso de erro.
 */
void set_sprite(int reg, int x, int y, int offset, int activation_bit);


/**
 * \brief Define um bloco de fundo.
 * 
 * \param[in] endereco_memoria : Endereço de memória do bloco de fundo
 * \param[in] R : Componente vermelho
 * \param[in] G : Componente verde
 * \param[in] B : Componente azul
 * 
 * \return Número de bytes escritos ou -1 em caso de erro.
 */
void set_background_block(int endereco_memoria, int R, int G, int B);


/**
 * \brief Define um polígono.
 * 
 * \param[in] forma : Forma do polígono
 * \param[in] R : Componente vermelho
 * \param[in] G : Componente verde
 * \param[in] B : Componente azul
 * \param[in] tamanho : Tamanho do polígono
 * \param[in] x : Posição x do polígono
 * \param[in] y : Posição y do polígono
 * \param[in] endereco : Endereço de memória do polígono
 * 
 * \return Número de bytes escritos ou -1 em caso de erro.
 */
void define_poligon(int forma, int R, int G, int B, int tamanho, int x, int y, int endereco);

/**
 * \brief Escreve um sprite na memória.
 * 
 * \param[in] R : Componente vermelho
 * \param[in] G : Componente verde
 * \param[in] B : Componente azul
 * \param[in] endereco_memoria : Endereço de memória onde o sprite será escrito
 * 
 * \return Número de bytes escritos ou -1 em caso de erro.
 */
void write_sprite_mem(int R, int G, int B, int endereco_memoria);
int write_to_bus(unsigned char *buffer);

void set_background_color(int R, int G, int B) {
    unsigned char buffer[MAX_SIZE]; /* Buffer que prepara um comando para configurar a cor de fundo com os valores fornecidos*/

    sprintf(buffer, "%d %d %d %d %d %d %d %d %d", WBR, R, G, B, 0x0000, 0, 0, 0, 0); /*Prepara o comando para configurar a cor de fundo*/
    write_to_bus(buffer);

}

void set_sprite(int reg, int x, int y, int offset, int activation_bit) {
    unsigned char buffer[MAX_SIZE];
    
    sprintf(buffer, "%d %d %d %d %d %d %d %d %d", WBR, 0, 0, 0, reg, x, y, offset, activation_bit); /*Prepara o comando para configurar o sprite*/
    write_to_bus(buffer);

}

void set_background_block(int endereco_memoria, int R, int G, int B) {
    unsigned char buffer[MAX_SIZE];

    sprintf(buffer, "%d %d %d %d %d", WBM, endereco_memoria, R, G, B); /* Prepara o comando para configurar o background block*/
    write_to_bus(buffer);

}

void define_poligon(int forma, int R, int G, int B, int tamanho, int x, int y, int endereco) {
    unsigned char buffer[MAX_SIZE];

    sprintf(buffer, "%d %d %d %d %d %d %d %d %d", DP, forma, R, G, B, tamanho, x, y, endereco); /*Prepara o comando para configurar o polígono*/
    write_to_bus(buffer);

}

void write_sprite_mem(int R, int G, int B, int endereco_memoria) {
    unsigned char buffer[MAX_SIZE];
    printf("R: %d, G: %d, B: %d, Endereço: %d\n", R, G, B, endereco_memoria); // Verifica todos os valores antes de formatar o buffer    sprintf(buffer, "%d %d %d %d %d", WSM, R, G, B, endereco_memoria);  /* Prepara o comando para escrever o sprite na memória*/
    write_to_bus(buffer);

}

int write_to_bus(unsigned char *buffer) {
    int fd;

    if ((fd = open("/dev/graphicProcessor", O_RDWR)) == -1) {
        printf("Error opening /dev/graphicProcessor: %s\n", strerror(errno));
        return -1;
    }

    int bytesWritten = write(fd, buffer, strlen(buffer)); /*Escreve o comando no dispositivo*/
    close(fd);

    return bytesWritten;

}

/**
 * Desativa cor de fundo para o original
*/
void clear_background_color(){
    set_background_color(0, 0, 0);
}

/**
 * Limpa os blocos 8x8 pixels da tela
*/
void clear_background_block() {
    int i;
    for(i = 0; i < 4800; i++) {
        set_background_block(i, 6, 7, 7);
    }
}

/**
 * Limpa todos os polígonos da tela
*/
void clear_poligonos(){
    int i;
    for (i = 0; i < 15; i++){
        define_poligon(i, 0, 0, 0, 0, 0, 0, 0);
    }
}

/**
 * Limpa os sprites renderizados na tela
*/
void clear_sprite() {
    int i;
    for (i = 1; i < 32; i++) {
        set_sprite(i, 0, 0, 0, 0);
    }
}

//step: novas coord 
void increase_coordinate(Sprite* sp, int step_x, int step_y) {
    sp->coord_x = step_x;
    sp->coord_y = step_y;
}

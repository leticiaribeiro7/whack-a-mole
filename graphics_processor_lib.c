#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include "headers/graphics_processor.h"

#define WBR      1 /*Operação para escrever cor de fundo*/
#define WBM      2 /*Operação para escrever bloco de fundo*/
#define DP       3 /*Operação para definir polígono*/
#define WSM      4 /*Operação para escrever sprite na memória*/

#define MAX_SIZE 32
#define SPRITE_SIZE 20

// https://www.istockphoto.com/br/vetor/pixel-de-brilho-colorido-bonito-pixel-cintilante-brilhante-estrela-brilhante-gm1501138740-522363421 sprites explosao


void set_background_color(uint8_t R, uint8_t G, uint8_t B) {
    unsigned char buffer[MAX_SIZE]; /* Buffer que prepara um comando para configurar a cor de fundo com os valores fornecidos*/
    sprintf(buffer, "%d %d %d %d %d %d %d %d %d", WBR, R, G, B, 0x0000, 0, 0, 0, 0); /*Prepara o comando para configurar a cor de fundo*/
    write_to_bus(buffer);

}

void set_sprite(uint8_t reg, uint16_t x, uint16_t y, uint16_t offset, uint8_t activation_bit) {
    unsigned char buffer[MAX_SIZE];
    
    sprintf(buffer, "%d %d %d %d %d %d %d %d %d", WBR, 0, 0, 0, reg, x, y, offset, activation_bit); /*Prepara o comando para configurar o sprite*/
    write_to_bus(buffer);

}

void set_background_block(uint16_t endereco_memoria, uint8_t R, uint8_t G, uint8_t B) {
    unsigned char buffer[MAX_SIZE];

    sprintf(buffer, "%d %d %d %d %d", WBM, endereco_memoria, R, G, B); /* Prepara o comando para configurar o background block*/
    write_to_bus(buffer);

}

void define_poligon(uint8_t forma, uint8_t R, uint8_t G, uint8_t B, uint8_t tamanho, uint16_t x, uint16_t y, uint8_t endereco) {
    unsigned char buffer[MAX_SIZE];

    sprintf(buffer, "%d %d %d %d %d %d %d %d %d", DP, forma, R, G, B, tamanho, x, y, endereco); /*Prepara o comando para configurar o polígono*/
    write_to_bus(buffer);

}

void write_sprite_mem(uint8_t R, uint8_t G, uint8_t B, uint16_t endereco_memoria) {
    unsigned char buffer[MAX_SIZE];
    printf("R: %d, G: %d, B: %d, Endereço: %d\n", R, G, B, endereco_memoria); // Verifica todos os valores antes de formatar o buffer    
    sprintf(buffer, "%d %d %d %d %d", WSM, R, G, B, endereco_memoria);  /* Prepara o comando para escrever o sprite na memória*/
    write_to_bus(buffer);

}

int write_to_bus(unsigned char *buffer) {
    int fd;

    if ((fd = open("/dev/graphic_processor", O_RDWR)) == -1) {
        printf("Error opening /dev/graphic_processor: %s\n", strerror(errno));
        return -1;
    }


    int bytesWritten = write(fd, buffer, strlen(buffer)); /*Escreve o comando no dispositivo*/
    close(fd);
    printf("Bytes escritos: %d", bytesWritten);

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

// Altera coordenadas do sprite
void change_coordinate(Sprite_Fixed* sp, int new_x, int new_y) {
    sp->coord_x = new_x;
    sp->coord_y = new_y;
}


int collision(Sprite* sp1, Sprite_Fixed* sp2) {
    return sp1->coord_x < sp2->coord_x + SPRITE_SIZE &&
           sp1->coord_x + SPRITE_SIZE > sp2->coord_x &&
           sp1->coord_y < sp2->coord_y + SPRITE_SIZE &&
           sp1->coord_y + SPRITE_SIZE > sp2->coord_y;
}

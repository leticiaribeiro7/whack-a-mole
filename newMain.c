#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include "spritesRGB.h"
#include "graphics_processor.h"

#define MOUSEFILE "/dev/input/mice"

void limitarCursor(int *x, int *y);

void limitarCursor(int *x, int *y) {
    if (*x <= 0) *x = 0;
    if (*y <= 0) *y = 0;
    if (*x >= 620) *x = 620;
    if (*y >= 461) *y = 461;
}

// Função que será executada na thread
void* movimentoToupeira(void* arg) {

    Sprite* toupeira = (Sprite*)arg;
    int max_y = 200;
    int min_y = 150;

    // int i;
    // for (i = 0; i < 400; i++) {
    //     int R = toupeiraSp[i][0];
    //     int G = toupeiraSp[i][1];
    //     int B = toupeiraSp[i][2];
    //     int endereco_memoria = 10400 + i;
    //     write_sprite_mem(R, G, B, endereco_memoria);
    // }

    // Sprite toupeira;
    // toupeira.coord_x = 100;
    // toupeira.coord_y = 150;
    // toupeira.offset = 26;
    // toupeira.data_register = 2;
    // toupeira.ativo = 1;
    // toupeira.collision = 0;
    // toupeira.direction = 1; // 1 ou -1

    while (1) {
        set_sprite(toupeira->data_register, toupeira->coord_x, toupeira->coord_y, toupeira->offset, toupeira->ativo);

        toupeira->coord_y += toupeira->direction * 5; // direction positivo: sobe, negativo: desce

        if (toupeira->coord_y >= max_y || toupeira->coord_y <= min_y) {
            toupeira->direction = -toupeira->direction; // Inverte a direção
        }

        sleep(1);
    }
    return NULL;
}

void* mouse(void* arg) {

    void** args = (void**)arg;
    Sprite_Fixed* martelo = (Sprite_Fixed*)args[0];
    Sprite* toupeira = (Sprite*)args[1];
    Sprite_Fixed* arbusto = (Sprite_Fixed*)args[2];

    int fd;
    int x = 0, y = 0;
    int leftButton;
    signed char x_disp, y_disp;
    char mouse_buffer[3];

    fd = open(MOUSEFILE, O_RDONLY); // Abre arquivo do mouse
    if (fd == -1) { // Se o arquivo do mouse retornar -1 significa que deu erro ao abrir o dispositivo
        perror("Não é possível abrir o dispositivo do mouse");
        exit(EXIT_FAILURE);
    }

    clear_sprite();

    // // Grava martelo
    // int i;
    // for (i = 0; i < 400; i++) {
    //     int R = marteloSp[i][0];
    //     int G = marteloSp[i][1];
    //     int B = marteloSp[i][2];
    //     int endereco_memoria = 10400 + i;
    //     write_sprite_mem(R, G, B, endereco_memoria);
    // }

    // Sprite_Fixed martelo;
    // martelo.coord_x = 0;
    // martelo.coord_y = 0;
    // martelo.offset = 25;
    // martelo.ativo = 1;
    // martelo.data_register = 1;

    while (1) {
        if (read(fd, &mouse_buffer, sizeof(mouse_buffer)) > 0) {
            system("clear");

            x_disp = mouse_buffer[1];
            y_disp = mouse_buffer[2];
            leftButton = mouse_buffer[0] & 0x1;

            x += x_disp;
            y -= y_disp;

            limitarCursor(&x, &y);
            set_sprite(arbusto->data_register, arbusto->coord_x, arbusto->coord_y, arbusto->offset, arbusto->ativo);

            set_sprite(martelo->data_register, x, y, martelo->offset, 1);
            martelo->coord_x = x;
            martelo->coord_y = y;

            if(collision(toupeira, martelo) && leftButton) {
                printf("1 ponto");
            }

            printf("Posição X: %d, Posição Y: %d\n", x, y);

        }
    }
    return NULL;
}

int main() {
    pthread_t thread1, thread2;

    Sprite toupeira;
    toupeira.coord_x = 100;
    toupeira.coord_y = 150;
    toupeira.offset = 26;
    toupeira.data_register = 3;
    toupeira.ativo = 1;
    toupeira.collision = 0;
    toupeira.direction = 1; // 1 ou -1


    Sprite_Fixed martelo;
    martelo.coord_x = 0;
    martelo.coord_y = 0;
    martelo.offset = 25;
    martelo.ativo = 1;
    martelo.data_register = 1;

    Sprite_Fixed arbusto;
    arbusto.coord_x = 300;
    arbusto.coord_y = 300;
    arbusto.offset = 27;
    arbusto.ativo = 1;
    arbusto.data_register = 2; 
// menor reg fica em cima

    // Grava martelo
    int i;
    for (i = 0; i < 400; i++) {
        int R = marteloSp[i][0];
        int G = marteloSp[i][1];
        int B = marteloSp[i][2];
        int endereco_memoria = 10000 + i;
        write_sprite_mem(R, G, B, endereco_memoria);
    }

    for (i = 0; i < 400; i++) {
        int R = toupeiraSp[i][0];
        int G = toupeiraSp[i][1];
        int B = toupeiraSp[i][2];
        int endereco_memoria = 10400 + i;
        write_sprite_mem(R, G, B, endereco_memoria);
    }

    for (i = 0; i < 400; i++) {
        int R = arbustoSp[i][0];
        int G = arbustoSp[i][1];
        int B = arbustoSp[i][2];
        int endereco_memoria = 10800 + i;
        write_sprite_mem(R, G, B, endereco_memoria);
    }


    void* args[3] = { &martelo, &toupeira, &arbusto };

    // Cria as threads
    if (pthread_create(&thread1, NULL, movimentoToupeira, (void*)&toupeira) != 0) {
        perror("Failed to create thread 1");
        return 1;
    }
    if (pthread_create(&thread2, NULL, mouse, (void*)args) != 0) {
        perror("Failed to create thread 2");
        return 1;
    }
    if (pthread_join(thread2, NULL) != 0) {
        perror("Failed to join thread 2");
        return 1;
    }

    return 0;
}

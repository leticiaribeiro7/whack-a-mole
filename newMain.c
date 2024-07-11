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
    int max_y = 200;
    int min_y = 150;

    int i;
    for (i = 0; i < 400; i++) {
        int R = toupeiraSp[i][0];
        int G = toupeiraSp[i][1];
        int B = toupeiraSp[i][2];
        int endereco_memoria = 10400 + i;
        write_sprite_mem(R, G, B, endereco_memoria);
    }

    Sprite toupeira;
    toupeira.coord_x = 100;
    toupeira.coord_y = 150;
    toupeira.offset = 26;
    toupeira.data_register = 2;
    toupeira.ativo = 1;
    toupeira.collision = 0;
    toupeira.direction = 1; // 1 ou -1

    while (1) {
        set_sprite(toupeira.data_register, toupeira.coord_x, toupeira.coord_y, toupeira.offset, toupeira.ativo);

        toupeira.coord_y += toupeira.direction * 5; // direction positivo: sobe, negativo: desce

        if (toupeira.coord_y >= max_y || toupeira.coord_y <= min_y) {
            toupeira.direction = -toupeira.direction; // Inverte a direção corretamente
        }

        sleep(1);
    }
    return NULL;
}

void* mouse(void* arg) {
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

    // Grava martelo
    int i;
    for (i = 0; i < 400; i++) {
        int R = marteloSp[i][0];
        int G = marteloSp[i][1];
        int B = marteloSp[i][2];
        int endereco_memoria = 10400 + i;
        write_sprite_mem(R, G, B, endereco_memoria);
    }

    Sprite_Fixed martelo;
    martelo.coord_x = 0;
    martelo.coord_y = 0;
    martelo.offset = 25;
    martelo.ativo = 1;
    martelo.data_register = 1;

    while (1) {
        if (read(fd, &mouse_buffer, sizeof(mouse_buffer)) > 0) {
            system("clear");

            x_disp = mouse_buffer[1];
            y_disp = mouse_buffer[2];
            leftButton = mouse_buffer[0] & 0x1;

            x += x_disp;
            y -= y_disp;

            limitarCursor(&x, &y);

            set_sprite(martelo.data_register, x, y, martelo.offset, 1);

            printf("Posição X: %d, Posição Y: %d\n", x, y);
        }
    }
    return NULL;
}

int main() {
    pthread_t thread1, thread2;

    // Cria as threads
    if (pthread_create(&thread1, NULL, movimentoToupeira, NULL) != 0) {
        perror("Failed to create thread 1");
        return 1;
    }
    if (pthread_create(&thread2, NULL, mouse, NULL) != 0) {
        perror("Failed to create thread 2");
        return 1;
    }

    // Espera pelas threads (não é necessário neste caso específico, mas é uma boa prática)
    if (pthread_join(thread1, NULL) != 0) {
        perror("Failed to join thread 1");
        return 1;
    }
    if (pthread_join(thread2, NULL) != 0) {
        perror("Failed to join thread 2");
        return 1;
    }

    return 0;
}

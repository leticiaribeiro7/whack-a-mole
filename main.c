#include "graphics_processor.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include "intelfpgaup/KEY.h"

int dataButton = 0;
volatile int button_pressed = 0;

#define MOUSEFILE "/dev/input/mice"


// Limitando coordenadas entre 1 e 511 - endereço do poligono tem 9 bits
void limitarCursor(int *x, int *y) {
    if (*x <= 1) *x = 1;
    if (*y <= 1) *y = 1;
    if (*x >= 511) *x = 511;
    if (*y >= 511) *y = 511;
}

void *button_detection(void *arg) {
    while (1) {

        if (dataButton == 0b0001) {
            button_pressed = 1;
        } else {
            button_pressed = 0;
        }
        usleep(10000);
    }
}



int main() {
    int fd;
    int x=0, y=0;
    int leftButton;
    signed char x_disp, y_disp;
    char mouse_buffer[3];


    // cria sprite de interrogação - matriz?
    //dividir valor rgb normal por 32 da o valor que da pra usar aqui

    int i;
    //linha superior
    for (i = 10000; i <= 10040; i++) {
        //write_sprite_mem(7, 7, 0, i);
    }

    //linha inferior
    for (i = 10360; i <= 10400; i++) {
        //write_sprite_mem(7, 7, 0, i);
    } 


    //lateral esquerda
    for (i = 10041; i <= 10341; i+=20) {
        //write_sprite_mem(7, 7, 0, i);
    }

    Sprite_Fixed sprite1;
    sprite1.coord_x = 0;
    sprite1.coord_y = 0;
    sprite1.offset = 25;
    sprite1.ativo = 1;
    sprite1.data_register = 1;

    //set_sprite(sprite1.data_register, sprite1.coord_x, sprite1.coord_y, sprite1.offset, sprite1.ativo);


    fd = open(MOUSEFILE, O_RDONLY); // Abre arquivo do mouse

    if (fd == -1) {// Se o arquivo do mouse retornar -1 significa que deu erro ao abrir o dispositivo
        perror("Não é possível abrir o dispositivo do mouse");
        exit(EXIT_FAILURE);
    }
    KEY_open(); // Abre botões da placa
    while(1) {
        if (read(fd, &mouse_buffer, sizeof(mouse_buffer)) > 0 ) { 
            define_poligon(1, 7, 1, 1, 0, x, y, 1); // limpa poligono
            x_disp = mouse_buffer[1];
            y_disp = mouse_buffer[2];
            leftButton = mouse_buffer[0] & 0x1;
            x += x_disp;
            y -= y_disp;
            limitarCursor(&x, &y);
            define_poligon(1, 7, 1, 1, 2, x, y, 1);

        // teste botão thread
            KEY_read(&dataButton);
              // Lógica principal do programa aqui
            if (button_pressed) {
                printf("Botão pressionado!\n");
                // Lógica adicional quando o botão é pressionado
                dataButton = 0; // Reseta o estado do botão após processamento
            }

            // Lógica do programa continua aqui
            usleep(50000); // Aguarda por 50ms antes de verificar novamente (ajuste conforme necessário)
        } 

    }

    KEY_close(); // fecha os botões da placa
}


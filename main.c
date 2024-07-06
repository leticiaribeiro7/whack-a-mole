#include "graphics_processor.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include "matrix.h"


#define MOUSEFILE "/dev/input/mice"


// coordenadas sprite: x e y ate 1024
void limitarCursor(int *x, int *y) {
    if (*x <= 0) *x = 0;
    if (*y <= 0) *y = 0;
    if (*x >= 620) *x = 620;
    if (*y >= 461) *y = 461;
}

int main() {
    int fd;
    int x=0, y=0;
    int leftButton;
    int x_disp, y_disp;
    char mouse_buffer[3];


    fd = open(MOUSEFILE, O_RDONLY); // Abre arquivo do mouse

    if (fd == -1) {// Se o arquivo do mouse retornar -1 significa que deu erro ao abrir o dispositivo
        perror("Não é possível abrir o dispositivo do mouse");
        exit(EXIT_FAILURE);
    }

    //grava martelo
   for (int i = 0; i < 400; i++) {
        write_sprite_mem(matrix[i][0], matrix[i][1], matrix[i][2], i);
    }

    Sprite_Fixed martelo;
    martelo.coord_x = 0;
    martelo.coord_y = 0;
    martelo.offset = 0;
    martelo.ativo = 1;
    martelo.data_register = 1;

    set_sprite(martelo.data_register, martelo.coord_x, martelo.coord_y, martelo.offset, martelo.ativo);
    
    while(1) {
        if (read(fd, &mouse_buffer, sizeof(mouse_buffer)) > 0 ) { 
            system("clear");
            //define_poligon(1, 7, 1, 1, 0, x, y, 1); // limpa poligono
            //clear_sprite();
            set_sprite(martelo.data_register, x, y, martelo.offset, 0);

            x_disp = mouse_buffer[1];
            y_disp = mouse_buffer[2];
            leftButton = mouse_buffer[0] & 0x1;

            x += x_disp;
            y -= y_disp;

            set_sprite(martelo.data_register, x, y, martelo.offset, martelo.ativo);
            limitarCursor(&x, &y);
            printf("Posição X: %d, Posição Y: %d\n", x, y);

            usleep(50000); // Aguarda por 50ms antes de verificar novamente (ajuste conforme necessário)
            }        
        }
    }

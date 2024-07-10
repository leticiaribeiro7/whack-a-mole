#include "graphics_processor.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include "spritesRGB.h"


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
    signed char x_disp, y_disp;
    char mouse_buffer[3];


    fd = open(MOUSEFILE, O_RDONLY); // Abre arquivo do mouse

    if (fd == -1) {// Se o arquivo do mouse retornar -1 significa que deu erro ao abrir o dispositivo
        perror("Não é possível abrir o dispositivo do mouse");
        exit(EXIT_FAILURE);
    }
    int i;

    clear_sprite();

    //grava martelo
    int j;
    j = 0;
    for (i = 10000; i < 10400; i++) {
        int R = martelo[j][0];
        int G = martelo[j][1];
        int B = martelo[j][2];
        j++;
        write_sprite_mem(R, G, B, i);
    }

    Sprite_Fixed martelo;
    martelo.coord_x = 0;
    martelo.coord_y = 0;
    martelo.offset = 25;
    martelo.ativo = 1;
    martelo.data_register = 1;

    Sprite sp_movel;
    sp_movel.coord_x = 100;
    sp_movel.coord_y = 150;
    sp_movel.offset = 9;
    sp_movel.data_register = 2;
    sp_movel.ativo = 1;
    sp_movel.collision = 0;

    //set_sprite(martelo.data_register, martelo.coord_x, martelo.coord_y, martelo.offset, martelo.ativo);
            int p;
            int q;
            p = 0; q = 0;

     while(1) {

        set_sprite(2, p, q, 3, 1);

        p++;
        q++;

        usleep(50000);

        }
    
    while(1) {

        if (read(fd, &mouse_buffer, sizeof(mouse_buffer)) > 0 ) { 
            system("clear");
            //define_poligon(1, 7, 1, 1, 0, x, y, 1); // limpa poligono

            //set_sprite(martelo.data_register, x, y, martelo.offset, 0);

            x_disp = mouse_buffer[1];
            y_disp = mouse_buffer[2];
            leftButton = mouse_buffer[0] & 0x1;

            x += x_disp;
            y -= y_disp;

            set_sprite(martelo.data_register, x, y, martelo.offset, 1);

            limitarCursor(&x, &y);


            if (collision(&sp_movel, &martelo)) {
                printf("Colisão detectada!\n");
                // Lógica adicional para tratar a colisão
            }

            printf("Posição X: %d, Posição Y: %d\n", x, y);

            //usleep(50000); // Aguarda por 50ms antes de verificar novamente (ajuste conforme necessário)
            }        
        }
    }

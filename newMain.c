#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include "spritesRGB.h"
#include "graphics_processor.h"
#include <sys/mman.h>
#include "address_map_arm.h"

#define MOUSEFILE "/dev/input/mice"
volatile int* KEY_ptr;

void limitarCursor(int *x, int *y) {
    if (*x <= 0) *x = 0;
    if (*y <= 0) *y = 0;
    if (*x >= 620) *x = 620;
    if (*y >= 461) *y = 461;
}


void* detectButton(void* arg) {
    
    int fd1 = -1; // used to open /dev/mem
    void *LW_virtual; // virtual address for light-weight bridge


    // Open /dev/mem to give access to physical addresses
    if ((fd1 = open("/dev/mem", (O_RDWR | O_SYNC))) == -1) {
        printf("ERROR: could not open \"/dev/mem\"\n");
        return -1;
    }

    // Get a mapping from physical addresses to virtual addresses
    LW_virtual = mmap(NULL, LW_BRIDGE_SPAN, (PROT_READ | PROT_WRITE), MAP_SHARED, fd1, LW_BRIDGE_BASE);

    if (LW_virtual == MAP_FAILED) {
        printf("ERROR: mmap() failed\n");
        close(fd1);
        return -1;
    }

    // Set virtual address pointer to I/O port (LEDR_BASE is assumed to be defined)
    KEY_ptr = (int *)(LW_virtual + KEYS_BASE);

    while (1) {
        if (*KEY_ptr == 0b0111) {
            
            set_background_color(7, 1, 1);
        } 
    }
}



// Função que será executada na thread
void* movimentoToupeira(void* arg) {
    void** args = (void**)arg;

    Sprite** toupeiras = (Sprite**)args[1];
    Sprite_Fixed** arbustos = (Sprite_Fixed**)args[2];

    // Sprite* toupeira2 = (Sprite*)args[3];
    // Sprite* toupeira3 = (Sprite*)args[4];
    // Sprite* toupeira4 = (Sprite*)args[5];
    // Sprite* toupeira5 = (Sprite*)args[6];

    int max_y = 200;
    int min_y = 150;


    // Sprite toupeira;
    // toupeira.coord_x = 100;
    // toupeira.coord_y = 150;
    // toupeira.offset = 26;
    // toupeira.data_register = 2;
    // toupeira.ativo = 1;
    // toupeira.collision = 0;
    // toupeira.direction = 1; // 1 ou -1

    while (1) {

        int i = 0;
        for (i; i < 9; i++) {
            set_sprite(arbustos[i]->data_register, arbustos[i]->coord_x, arbustos[i]->coord_y, arbustos[i]->offset, arbustos[i]->ativo);

            set_sprite(toupeiras[i]->data_register, toupeiras[i]->coord_x, toupeiras[i]->coord_y, toupeiras[i]->offset, toupeiras[i]->ativo);

            toupeiras[i]->coord_y += toupeiras[i]->direction * 5; // direction positivo: sobe, negativo: desce

            if (i >= 0 || i <= 2) {
                min_y = 250;
                max_y = 300;
            } 
            if (i <= 3 || i >= 5) {
                min_y = 350;
                max_y = 400;
            }
            if ( i <= 6 || i <= 8) {
                min_y = 450;
                max_y = 500;
            }

            if (toupeiras[i]->coord_y >= max_y || toupeiras[i]->coord_y <= arbustos[i]->coord_y) {
                toupeiras[i]->direction = -toupeiras[i]->direction; // Inverte a direção
            }
        }


        usleep(50000);
    }
    return NULL;
}

void* mouse(void* arg) {

    void** args = (void**)arg;
    Sprite_Fixed* martelo = (Sprite_Fixed*)args[0];
    Sprite** toupeiras = (Sprite**)args[1];
    

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
    //clear_background_block();
    clear_background_color();




    while (1) {
        if (read(fd, &mouse_buffer, sizeof(mouse_buffer)) > 0) {
            system("clear");

            x_disp = mouse_buffer[1];
            y_disp = mouse_buffer[2];
            leftButton = mouse_buffer[0] & 0x1;

            x += x_disp;
            y -= y_disp;

            limitarCursor(&x, &y);
            

            set_sprite(martelo->data_register, x, y, martelo->offset, 1);
            martelo->coord_x = x;
            martelo->coord_y = y;


            int i;
            for (i = 0; i < 9; i++) {
                if(collision(toupeiras[i], martelo) && leftButton) {
                    printf("1 ponto");
                }
            }

            printf("Posição X: %d, Posição Y: %d\n", x, y);
            printf("botao: %d", *KEY_ptr); // tem logica invertida

        }
    }
    return NULL;
}

int main() {
    pthread_t thread1, thread2, thread3;

    //set_background_color(1, 4, 7);


//primeira linha
    Sprite toupeira1;
    toupeira1.coord_x = 200;
    toupeira1.coord_y = 250;
    toupeira1.offset = 26;
    toupeira1.data_register = 11;
    toupeira1.ativo = 1;
    toupeira1.collision = 0;
    toupeira1.direction = 1; // 1 ou -1

    Sprite toupeira2;
    toupeira2.coord_x = 300;
    toupeira2.coord_y = 250;
    toupeira2.offset = 26;
    toupeira2.data_register = 12;
    toupeira2.ativo = 1;
    toupeira2.collision = 0;
    toupeira2.direction = 1; // 1 ou -1

    Sprite toupeira3;
    toupeira3.coord_x = 400;
    toupeira3.coord_y = 250;
    toupeira3.offset = 26;
    toupeira3.data_register = 13;
    toupeira3.ativo = 1;
    toupeira3.collision = 0;
    toupeira3.direction = 1; // 1 ou -1


// segunda linha
    Sprite toupeira4;
    toupeira4.coord_x = 200;
    toupeira4.coord_y = 350;
    toupeira4.offset = 26;
    toupeira4.data_register = 14;
    toupeira4.ativo = 1;
    toupeira4.collision = 0;
    toupeira4.direction = 1; // 1 ou -1

    Sprite toupeira5;
    toupeira5.coord_x = 300;
    toupeira5.coord_y = 350;
    toupeira5.offset = 26;
    toupeira5.data_register = 15;
    toupeira5.ativo = 1;
    toupeira5.collision = 0;
    toupeira5.direction = 1; // 1 ou -1


    Sprite toupeira6;
    toupeira6.coord_x = 400;
    toupeira6.coord_y = 350;
    toupeira6.offset = 26;
    toupeira6.data_register = 16;
    toupeira6.ativo = 1;
    toupeira6.collision = 0;
    toupeira6.direction = 1; // 1 ou -1

    // terceira linha
    Sprite toupeira7;
    toupeira7.coord_x = 200;
    toupeira7.coord_y = 450;
    toupeira7.offset = 26;
    toupeira7.data_register = 17;
    toupeira7.ativo = 1;
    toupeira7.collision = 0;
    toupeira7.direction = 1; // 1 ou -1

    Sprite toupeira8;
    toupeira8.coord_x = 300;
    toupeira8.coord_y = 450;
    toupeira8.offset = 26;
    toupeira8.data_register = 18;
    toupeira8.ativo = 1;
    toupeira8.collision = 0;
    toupeira8.direction = 1; // 1 ou -1

    Sprite toupeira9;
    toupeira9.coord_x = 400;
    toupeira9.coord_y = 450;
    toupeira9.offset = 26;
    toupeira9.data_register = 19;
    toupeira9.ativo = 1;
    toupeira9.collision = 0;
    toupeira9.direction = 1; // 1 ou -1



    Sprite* toupeiras[9] = {
        &toupeira1,
        &toupeira2,
        &toupeira3,
        &toupeira4,
        &toupeira5,
        &toupeira6,
        &toupeira7,
        &toupeira8,
        &toupeira9
    };



    Sprite_Fixed martelo;
    martelo.coord_x = 0;
    martelo.coord_y = 0;
    martelo.offset = 25;
    martelo.ativo = 1;
    martelo.data_register = 1;

//Arbusto linha 1
    Sprite_Fixed arbusto1;
    arbusto1.coord_x = 200;
    arbusto1.coord_y = 250;
    arbusto1.offset = 27;
    arbusto1.ativo = 1;
    arbusto1.data_register = 2; 

    Sprite_Fixed arbusto2;
    arbusto2.coord_x = 300;
    arbusto2.coord_y = 250;
    arbusto2.offset = 27;
    arbusto2.ativo = 1;
    arbusto2.data_register = 3; 

    Sprite_Fixed arbusto3;
    arbusto3.coord_x = 400;
    arbusto3.coord_y = 250;
    arbusto3.offset = 27;
    arbusto3.ativo = 1;
    arbusto3.data_register = 4; 
// linha 2
    Sprite_Fixed arbusto4;
    arbusto4.coord_x = 200;
    arbusto4.coord_y = 300;
    arbusto4.offset = 27;
    arbusto4.ativo = 1;
    arbusto4.data_register = 5; 

    Sprite_Fixed arbusto5;
    arbusto5.coord_x =  300;
    arbusto5.coord_y = 300;
    arbusto5.offset = 27;
    arbusto5.ativo = 1;
    arbusto5.data_register = 6; 

    Sprite_Fixed arbusto6;
    arbusto6.coord_x = 400;
    arbusto6.coord_y = 300;
    arbusto6.offset = 27;
    arbusto6.ativo = 1;
    arbusto6.data_register = 7; 
//linha 3
    Sprite_Fixed arbusto7;
    arbusto7.coord_x = 200;
    arbusto7.coord_y = 350;
    arbusto7.offset = 27;
    arbusto7.ativo = 1;
    arbusto7.data_register = 8; 

    Sprite_Fixed arbusto8;
    arbusto8.coord_x = 300;
    arbusto8.coord_y = 350;
    arbusto8.offset = 27;
    arbusto8.ativo = 1;
    arbusto8.data_register = 9; 

    Sprite_Fixed arbusto9;
    arbusto9.coord_x = 400;
    arbusto9.coord_y = 350;
    arbusto9.offset = 27;
    arbusto9.ativo = 1;
    arbusto9.data_register = 10; 
    
    Sprite_Fixed* arbustos[9] = {
    &arbusto1,
    &arbusto2,
    &arbusto3,
    &arbusto4,
    &arbusto5,
    &arbusto6,
    &arbusto7,
    &arbusto8,
    &arbusto9
};

// menor reg fica em cima

    // Grava martelo
    int i;
    for (i = 0; i < 380; i++) {
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
        int R = arbusto[i][0];
        int G = arbusto[i][1];
        int B = arbusto[i][2];
        int endereco_memoria = 10800 + i;
        write_sprite_mem(R, G, B, endereco_memoria);
    }


    void* args[3] = { &martelo, &toupeiras, &arbustos};

    // Cria as threads
    //if (pthread_create(&thread1, NULL, movimentoToupeira, (void*)&toupeira) != 0) {
    if (pthread_create(&thread1, NULL, movimentoToupeira, (void*)args) != 0) {
        perror("Failed to create thread 1");
        return 1;
    }
    if (pthread_create(&thread2, NULL, mouse, (void*)args) != 0) {
        perror("Failed to create thread 2");
        return 1;
    }
    if (pthread_create(&thread3, NULL, detectButton, NULL) != 0) {
        perror("Failed to create thread 3");
        return 1;
    }
    if (pthread_join(thread1, NULL) != 0) {
        perror("Failed to join thread 1");
        return 1;
    }
    if (pthread_join(thread2, NULL) != 0) {
        perror("Failed to join thread 2");
        return 1;
    }
    if (pthread_join(thread3, NULL) != 0) {
        perror("Failed to join thread 3");
        return 1;
    }

    return 0;
}

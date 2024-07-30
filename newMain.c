#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/mman.h>
#include <stdint.h>

#include "headers/screens.h"
#include "headers/utils.h"
#include "headers/screensRGB.h"
#include "headers/spritesRGB.h"
#include "headers/graphics_processor.h"
#include "headers/address_map_arm.h"

#define MOUSEFILE "/dev/input/mice"

#define START 0
#define RUNNING 1
#define PAUSED 2
#define ENDED_BY_TIME 3
#define ENDED_BY_BUTTON 4


extern volatile int* HEX0_ptr;
extern volatile int* HEX1_ptr;
extern volatile int* HEX2_ptr;
extern volatile int* HEX3_0_ptr;
extern volatile int* KEY_ptr;

int button0, button1, button2, button3;
int gameStarted = 0; int paused = 0; int lido = 0;
int state = 0; // pra acontecer as trocas de tela de acordo ao estado
int pontuacao = 0;


int segmentos[10] = {
    0b1000000,
    0b1111001,
    0b0100100,
    0b0110000,
    0b0011001,
    0b0010010,
    0b0000010,
    0b1111000,
    0b0000000,
    0b0010000
};

void readButtons() {
    button0 = ((*KEY_ptr & 0b0001) == 0); // iniciar
    button1 = ((*KEY_ptr & 0b0010) == 0); // pausar
    button2 = ((*KEY_ptr & 0b0100) == 0); // reiniciar
    button3 = ((*KEY_ptr & 0b1000) == 0); // parar
}


void draw_game_screen() {
    int i;
    for (i = 0; i < 4800; i++) {
        int R = gameNew[i][0];
        int G = gameNew[i][1];
        int B = gameNew[i][2];
        set_background_block(i, R, G, B);
    }
}
// column + line*80
void draw_game_over_screen() {

}

void remove_pause_blocks() {
    set_background_block(162, 0, 5, 7);
    set_background_block(164, 0, 5, 7);
    set_background_block(242, 0, 5, 7);
    set_background_block(244, 0, 5, 7);
    set_background_block(322, 0, 5, 7);
    set_background_block(324, 0, 5, 7);
}

void draw_pause_blocks() {
    set_background_block(162, 7, 1, 1);
    set_background_block(164, 7, 1, 1);
    set_background_block(242, 7, 1, 1);
    set_background_block(244, 7, 1, 1);
    set_background_block(322, 7, 1, 1);
    set_background_block(324, 7, 1, 1);
}


void* movimentoToupeira(void* arg) {
    void** args = (void**)arg;

    Sprite** toupeiras = (Sprite**)args[1];
    Sprite_Fixed** arbustos = (Sprite_Fixed**)args[2];

    srand(time(NULL));

    int start_time = 0;
    int pause_time = 0;
    int total_pause_time = 0;
    int last_check_time = 0;

    uint16_t base_block_address = 315; // teste

    start_time = time(NULL);
    last_check_time = time(NULL);

    while (1) {
        // Escuta os botões
       // readButtons();

        if (button0 && state == START) { // inicia se ainda nao tiver iniciado
            clear_background_block();
            
            draw_game_screen();
            gameStarted = 1;
            state = RUNNING; // rodando
        }

        if (button1 && state == RUNNING) { // so pausa se tiver rodando
            state = PAUSED; //pausado
            draw_pause_blocks();
            pause_time = time(NULL);
        }

        while (button1) {
            readButtons(); /* aguarda soltar o botão */
        }

        readButtons();

        if (button1 && state == PAUSED) { // retorna da pausa se tiver pausado
            state = RUNNING;
            remove_pause_blocks();
            total_pause_time += time(NULL) - pause_time;
        }

        if (button2) { // restart
            draw_game_screen(); // redesenha a tela do game
            state = RUNNING;
            pontuacao = 0; // reinicia pontuação
            start_time = time(NULL); // reinicia o tempo
            last_check_time = time(NULL);
        }

        if (button3) { // encerra em qualquer state
            state = ENDED_BY_BUTTON; //encerrado
            //clear_background_block();
            draw_game_over_screen();
            break;
        }


        if (state == RUNNING) {

            int current_time = time(NULL);
            int i;
            for (i = 0; i < 9; i++) {
                //printf("Toupeira %d: coord_y = %d, direction = %d, moving = %d, last_update = %d, interval = %d\n", i, toupeiras[i]->coord_y, toupeiras[i]->direction, toupeiras[i]->moving, toupeiras[i]->last_update, toupeiras[i]->interval);                
                if (toupeiras[i]->moving) {
                    // Movimenta a toupeira
                    toupeiras[i]->coord_y -= toupeiras[i]->direction * 5; // pra cima diminui
                    toupeiras[i]->interval = rand() % 3 + 1; // entre 1 e 3 seg

                    // Verifica se chegou ao limite e inverte a direção
                    // Ta em cima e desce
                    if (toupeiras[i]->coord_y <= toupeiras[i]->max_y) {
                        toupeiras[i]->direction = -1;
                    // Ta embaixo e para
                    } else if (toupeiras[i]->coord_y >= toupeiras[i]->min_y) {
                        toupeiras[i]->direction = 1;
                        toupeiras[i]->moving = 0;
                        toupeiras[i]->last_update = current_time; // Atualiza o tempo da última parada
                    }
                    set_sprite(arbustos[i]->data_register, arbustos[i]->coord_x, arbustos[i]->coord_y, arbustos[i]->offset, arbustos[i]->ativo);
                    set_sprite(toupeiras[i]->data_register, toupeiras[i]->coord_x, toupeiras[i]->coord_y, toupeiras[i]->offset, toupeiras[i]->ativo);

                    //printf("Toupeira %d movendo para coord_y = %d, direction = %d\n", i, toupeiras[i]->coord_y, toupeiras[i]->direction);
                    // Atualiza o sprite
                } else if (current_time - toupeiras[i]->last_update >= toupeiras[i]->interval) {
                    // Define um novo intervalo aleatório
                    toupeiras[i]->moving = 1; // Retoma o movimento da toupeira
                    //printf("Toupeira %d retoma movimento com novo intervalo = %d\n", i, toupeiras[i]->interval);
                }
                readButtons();
            }

            if (pontuacao < 25) {
                usleep(300000); // 250ms
            } else if (pontuacao < 50) {
                usleep(200000); // 150 ms
            } // mov mais rapido com maior pontuação

            if ((current_time - last_check_time) >= 5) {
                printf("Verificação a cada 5 segundos: %d segundos decorridos\n", current_time - start_time - total_pause_time);

                set_background_block(base_block_address, 0, 5, 7);
                base_block_address -= 1;
                last_check_time = current_time; // Atualiza o tempo da última verificação
            } // cada 5 seg

            if ((current_time - start_time - total_pause_time) >= 45) {
                state = ENDED_BY_TIME;
                draw_game_over_screen();
            }
        }
    }
}

uint8_t display(int number) {
    return segmentos[number];
    //return segmentos_map;
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
    if (fd == -1) { // Se o arquivo do mouse retornar -1 significa que deu erro
        perror("Não é possível abrir o dispositivo do mouse");
        exit(EXIT_FAILURE);
    }

    while(1) {

        if (read(fd, &mouse_buffer, sizeof(mouse_buffer)) > 0) {
            system("clear");

            x_disp = mouse_buffer[1];
            y_disp = mouse_buffer[2];
            leftButton = mouse_buffer[0] & 0x1;

            x += x_disp;
            y -= y_disp;

            limitarCursor(&x, &y);


            set_sprite(martelo->data_register, x, y, martelo->offset, 1);
            change_coordinate(martelo, x, y);
            // martelo->coord_x = x;
            // martelo->coord_y = y;


            int i;
            for (i = 0; i < 9; i++) {
                 if (collision(toupeiras[i], martelo) && leftButton && toupeiras[i]->coord_y <= toupeiras[i]->max_y && !paused) {
                    pontuacao += 1;

                }
            }

            /* ======= DISPLAY ======== */


            /* Formatação da pontuação pra o display 7 */
            int dezena = pontuacao / 10;
            int unidade = pontuacao % 10;
            int centena = (pontuacao / 100) % 10;


            *HEX0_ptr = display(unidade);
            *HEX1_ptr = display(dezena);
            *HEX2_ptr = display(centena);

        }   
        if (state == ENDED_BY_BUTTON) {
            break;
        }

    }


    return NULL;
}

void write_sprites() {

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
        int R = arbustoSp[i][0];
        int G = arbustoSp[i][1];
        int B = arbustoSp[i][2];
        int endereco_memoria = 10800 + i;
        write_sprite_mem(R, G, B, endereco_memoria);
    }
}


void draw_initial_screen() {
    int i;
    for (i = 0; i < 4800; i++) {
        int R = initialScreen[i][0];
        int G = initialScreen[i][1];
        int B = initialScreen[i][2];
        set_background_block(i, R, G, B);
    }
}

int main() {
    
    pthread_t thread1, thread2;

    mapPeripherals();
    clear_sprite();
    draw_initial_screen();


        // Grava martelo
    int i;
    for (i = 0; i < 380; i++) {
        int R = marteloSp[i][0];
        int G = marteloSp[i][1];
        int B = marteloSp[i][2];
        int endereco_memoria = 10000 + i;
        //pthread_mutex_lock(&mutex);
        write_sprite_mem(R, G, B, endereco_memoria);
        //pthread_mutex_unlock(&mutex);
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


/* ============ CRIAÇÃO DE SPRITES ============= */

// primeira linha
    Sprite toupeira1;
    toupeira1.coord_x = 200;
    toupeira1.coord_y = 250;
    toupeira1.offset = 26;
    toupeira1.data_register = 11;
    toupeira1.ativo = 1;
    toupeira1.direction = 1; // 1 ou -1
    toupeira1.moving = 1;
    toupeira1.min_y = 250;
    toupeira1.max_y = 235;

    Sprite toupeira2;
    toupeira2.coord_x = 300;
    toupeira2.coord_y = 250;
    toupeira2.offset = 26;
    toupeira2.data_register = 12;
    toupeira2.ativo = 1;
    toupeira2.direction = 1; // 1 ou -1
    toupeira2.moving = 1;
    toupeira2.min_y = 250;
    toupeira2.max_y = 235;

    Sprite toupeira3;
    toupeira3.coord_x = 400;
    toupeira3.coord_y = 250;
    toupeira3.offset = 26;
    toupeira3.data_register = 13;
    toupeira3.ativo = 1;
    toupeira3.direction = 1; // 1 ou -1
    toupeira3.moving = 1;
    toupeira3.min_y = 250;
    toupeira3.max_y = 235;

// segunda linha
    Sprite toupeira4;
    toupeira4.coord_x = 200;
    toupeira4.coord_y = 300;
    toupeira4.offset = 26;
    toupeira4.data_register = 14;
    toupeira4.ativo = 1;
    toupeira4.direction = 1; // 1 ou -1
    toupeira4.moving = 1;
    toupeira4.min_y = 300;
    toupeira4.max_y = 285;

    Sprite toupeira5;
    toupeira5.coord_x = 300;
    toupeira5.coord_y = 300;
    toupeira5.offset = 26;
    toupeira5.data_register = 15;
    toupeira5.ativo = 1;
    toupeira5.direction = 1; // 1 ou -1
    toupeira5.moving = 1;
    toupeira5.min_y = 300;
    toupeira5.max_y = 285;

    Sprite toupeira6;
    toupeira6.coord_x = 400;
    toupeira6.coord_y = 300;
    toupeira6.offset = 26;
    toupeira6.data_register = 16;
    toupeira6.ativo = 1;
    toupeira6.direction = 1; // 1 ou -1
    toupeira6.moving = 1;
    toupeira6.min_y = 300;
    toupeira6.max_y = 285;
    // terceira linha
    Sprite toupeira7;
    toupeira7.coord_x = 200;
    toupeira7.coord_y = 350;
    toupeira7.offset = 26;
    toupeira7.data_register = 17;
    toupeira7.ativo = 1;
    toupeira7.direction = 1; // 1 ou -1
    toupeira7.moving = 1;
    toupeira7.min_y = 350;
    toupeira7.max_y = 335;

    Sprite toupeira8;
    toupeira8.coord_x = 300;
    toupeira8.coord_y = 350;
    toupeira8.offset = 26;
    toupeira8.data_register = 18;
    toupeira8.ativo = 1;
    toupeira8.direction = 1; // 1 ou -1
    toupeira8.moving = 1;
    toupeira8.min_y = 350;
    toupeira8.max_y = 335;

    Sprite toupeira9;
    toupeira9.coord_x = 400;
    toupeira9.coord_y = 350;
    toupeira9.offset = 26;
    toupeira9.data_register = 19;
    toupeira9.ativo = 1;
    toupeira9 ;
    toupeira9.direction = 1; // 1 ou -1
    toupeira9.moving = 1;
    toupeira9.min_y = 350;
    toupeira9.max_y = 335;

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

    //write_sprites();

    void* args[3] = { &martelo, &toupeiras, &arbustos};

    
    
  
/*============== CRIAÇÃO DE THREADS ================== */
    if (pthread_create(&thread1, NULL, movimentoToupeira, (void*)args) != 0) {
        perror("Failed to create thread 1");
        return 1;
    }
    if (pthread_create(&thread2, NULL, mouse, (void*)args) != 0) {
        perror("Failed to create thread 2");
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
    //if (pthread_create(&thread1, NULL, movimentoToupeira, (void*)&toupeira) != 0) {
    // if (pthread_join(thread3, NULL) != 0) {
    //     perror("Failed to join thread 3");
    //     return 1;
    // }

    return 0;
}

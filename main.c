#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/mman.h>
#include <stdint.h>

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
extern volatile int* HEX3_ptr;
extern volatile int* HEX4_ptr;
extern volatile int* HEX5_ptr;

extern volatile int* KEY_ptr;

extern int button0, button1, button2, button3;
int state = 0; /* Controladora de estado*/
int pontuacao = 0;


/**
 * \brief Função que desenha a tela inicial
 * 
 * Esta função percorre um array que contém os valores RGB para 
 * cada pixel na tela de início. Em seguida, chama a função set_background_block 
 * para definir a cor de fundo de cada bloco na tela de acordo com os valores RGB.
 */
void draw_initial_screen() {
    int i;
    for (i = 0; i < 4800; i++) {
        int R = initialScreen[i][0];
        int G = initialScreen[i][1];
        int B = initialScreen[i][2];
        set_background_block(i, R, G, B);
    }
}

/**
 * \brief Função que desenha a tela do jogo
 * 
 * Esta função percorre um array que contém os valores RGB para 
 * cada pixel na tela do jogo. Em seguida, chama a função set_background_block 
 * para definir a cor de fundo de cada bloco na tela de acordo com os valores RGB.
 */
void draw_game_screen() {
    int i;
    for (i = 0; i < 4800; i++) {
        int R = gameScreen[i][0];
        int G = gameScreen[i][1];
        int B = gameScreen[i][2];
        set_background_block(i, R, G, B);
    }
}

/**
 * \brief Função que desenha a tela de "game over"
 * 
 * Esta função percorre um array que contém os valores RGB para 
 * cada pixel na tela de game over. Em seguida, chama a função set_background_block 
 * para definir a cor de fundo de cada bloco na tela de acordo com os valores RGB.
 */
// column + line*80
void draw_game_over_screen() {
    int i;
    for (i = 0; i < 4800; i++) {
        int R = gameOver[i][0];
        int G = gameOver[i][1];
        int B = gameOver[i][2];
        set_background_block(i, R, G, B);
    }
}

/**
 * \brief Função que remove os blocos de pausa
 * 
 * Esta função define os blocos de fundo nas posições especificadas para as 
 * cores fornecidas (1, 4, 6), removendo assim a indicação de pausa na tela.
 */
void remove_pause_blocks() {
    set_background_block(162, 1, 4, 6);
    set_background_block(164, 1, 4, 6);
    set_background_block(242, 1, 4, 6);
    set_background_block(244, 1, 4, 6);
    set_background_block(322, 1, 4, 6);
    set_background_block(324, 1, 4, 6);
}

/**
 * \brief Função que desenha os blocos de pausa
 * 
 * Esta função define os blocos de fundo nas posições especificadas para as 
 * cores fornecidas (7, 1, 1), indicando assim a pausa na tela.
 */
void draw_pause_blocks() {
    set_background_block(162, 7, 1, 1);
    set_background_block(164, 7, 1, 1);
    set_background_block(242, 7, 1, 1);
    set_background_block(244, 7, 1, 1);
    set_background_block(322, 7, 1, 1);
    set_background_block(324, 7, 1, 1);
}


/**
 * \brief Função que controla o movimento das toupeiras
 * 
 * Esta função é executada em uma thread separada e controla o movimento das toupeiras 
 * no jogo. Ela utiliza um array de sprites de toupeiras e arbustos. Além disso, movimenta as 
 * toupeiras de acordo com um intervalo aleatório de tempo.
 * 
 * \param[in] arg : Argumentos passados para a thread, contendo os arrays de toupeiras e arbustos
 * \return NULL
 */
void* movimentoToupeira(void* arg) {
    void** args = (void**)arg;

    Sprite** toupeiras = (Sprite**)args[1];
    Sprite_Fixed** arbustos = (Sprite_Fixed**)args[2];

    srand(time(NULL));

    int start_time = 0;
    int pause_time = 0;
    int total_pause_time = 0;
    int last_check_time = 0;
    

    uint16_t base_block_address = 315; /* Endereço do último bloco na barra de tempo */

  
    while (1) {

        readButtons();

        /**
         * \brief Inicia o jogo se o botão de iniciar for pressionado e o estado for START
         * 
         * Quando o botão de iniciar (button0) é pressionado e o estado atual é START, 
         * a função limpa o bloco de fundo, desenha a tela do jogo e define o estado para RUNNING.
         * O tempo de início e o último tempo de verificação são registrados.
         * \param[in]       button0: Estado do botão de iniciar
         * \param[in]       state: Estado atual do jogo
         * \param[out]      state: Novo estado do jogo após a ação
         * \param[out]      start_time: Tempo de início do jogo
         * \param[out]      last_check_time: Último tempo de verificação do jogo
         */
        if (button0 && state == START) {
            clear_background_block();
            draw_game_screen();
            state = RUNNING;
            start_time = time(NULL);
            last_check_time = time(NULL);
        }

        /**
         * \brief Pausa o jogo se o botão de pausar for pressionado e o estado for RUNNING
         * 
         * Quando o botão de pausar (button1) é pressionado e o estado atual é RUNNING, 
         * a função define o estado para PAUSED, desenha os blocos de pausa e registra o tempo de pausa.
         * \param[in]       button1: Estado do botão de pausar
         * \param[in]       state: Estado atual do jogo
         * \param[out]      state: Novo estado do jogo após a ação
         * \param[out]      pause_time: Tempo de início da pausa
         */
        if (button1 && state == RUNNING) {
            state = PAUSED;
            draw_pause_blocks();
            pause_time = time(NULL);
        }

        while (button1) {
            readButtons(); /* aguarda soltar o botão */
        }

        readButtons();

        /**
         * \brief Retorna do estado de pausa se o botão de pausar for pressionado novamente e o estado for PAUSED
         * 
         * Quando o botão de pausar (button1) é pressionado novamente e o estado atual é PAUSED, 
         * a função define o estado para RUNNING, remove os blocos de pausa e ajusta o tempo total de pausa.
         * \param[in]       button1: Estado do botão de pausar
         * \param[in]       state: Estado atual do jogo
         * \param[out]      state: Novo estado do jogo após a ação
         * \param[out]      total_pause_time: Tempo total de pausa ajustado
         */
        if (button1 && state == PAUSED) {
            state = RUNNING;
            remove_pause_blocks();
            total_pause_time += time(NULL) - pause_time;
        }

        /**
         * \brief Reinicia o jogo se o botão de reiniciar for pressionado
         * 
         * Quando o botão de reiniciar (button2) é pressionado, a função redefine o endereço do bloco base, 
         * define o estado para RUNNING, reinicia a pontuação, o tempo total de pausa, o tempo de início e o 
         * último tempo de verificação. Em seguida, redesenha a tela do jogo.
         * \param[in]       button2: Estado do botão de reiniciar
         * \param[out]      base_block_address: Novo endereço do bloco base
         * \param[out]      state: Novo estado do jogo após a ação
         * \param[out]      pontuacao: Pontuação reiniciada
         * \param[out]      total_pause_time: Tempo total de pausa reiniciado
         * \param[out]      start_time: Tempo de início reiniciado
         * \param[out]      last_check_time: Último tempo de verificação reiniciado
         */
        if (button2) {
            base_block_address = 315;
            state = RUNNING;
            pontuacao = 0;
            total_pause_time = 0;
            start_time = time(NULL);
            last_check_time = start_time;
            draw_game_screen();
        }

        /**
         * \brief Encerra o jogo se o botão de parar for pressionado
         * 
         * Quando o botão de parar (button3) é pressionado em qualquer estado, 
         * a função define o estado para ENDED_BY_BUTTON, limpa os sprites e o bloco de fundo, e sai do loop.
         * \param[in]       button3: Estado do botão de parar
         * \param[out]      state: Novo estado do jogo após a ação
         */
        if (button3) {
            state = ENDED_BY_BUTTON;
            clear_sprite();
            clear_background_block();
            break;
        }


        if (state == RUNNING) {

            int current_time = time(NULL);
            int i;

            /**
             * \brief Laço para controlar o movimento das toupeiras
             * 
             * Este laço percorre todas as toupeiras, verificando se elas estão em movimento. 
             * Se estiverem, a função atualiza a posição da toupeira e verifica se ela chegou ao 
             * limite, invertendo a direção ou parando o movimento conforme necessário.
             * Se a toupeira não estiver em movimento, verifica se é hora de retomar o movimento 
             * com base no intervalo definido.
             * \param[in]       toupeiras: Array de toupeiras
             * \param[in]       current_time: Tempo atual
             * \param[out]      toupeiras[i]: Posição e estado atualizado da toupeira
             */
            for (i = 0; i < 9; i++) {
                if (toupeiras[i]->moving) {
                    /* Movimenta a toupeira*/
                    toupeiras[i]->coord_y -= toupeiras[i]->direction * 5; /*pra cima diminui*/
                    toupeiras[i]->interval = rand() % 3 + 1; /*entre 1 e 3 seg*/

                    /*Verifica se chegou ao limite e inverte a direção*/
                    if (toupeiras[i]->coord_y <= toupeiras[i]->max_y) {
                        toupeiras[i]->direction = -1;
                    } else if (toupeiras[i]->coord_y >= toupeiras[i]->min_y) {
                        toupeiras[i]->direction = 1;
                        toupeiras[i]->moving = 0;
                        toupeiras[i]->last_update = current_time; /*Atualiza o tempo da última parada*/
                    }
                    set_sprite(arbustos[i]->data_register, arbustos[i]->coord_x, arbustos[i]->coord_y, arbustos[i]->offset, arbustos[i]->ativo);
                    set_sprite(toupeiras[i]->data_register, toupeiras[i]->coord_x, toupeiras[i]->coord_y, toupeiras[i]->offset, toupeiras[i]->ativo);
                } else if (current_time - toupeiras[i]->last_update >= toupeiras[i]->interval) {
                    /*Define um novo intervalo aleatório*/
                    toupeiras[i]->moving = 1; /*Retoma o movimento da toupeira*/
                   
                }
                readButtons();
            }

            /* Movimenta mais rápido quando a pontuação é maior que 25 */
            if (pontuacao < 25) {
                usleep(400000); // 400 ms
            } else if (pontuacao < 50) {
                usleep(300000); // 300 ms
            }


            /**
             * \brief Atualiza o bloco de fundo periodicamente
             * 
             * A cada 5 segundos, a função atualiza o bloco de fundo e ajusta o endereço do bloco base.
             * \param[in]       current_time: Tempo atual
             * \param[in]       last_check_time: Último tempo de verificação
             * \param[out]      base_block_address: Novo endereço do bloco base
             * \param[out]      last_check_time: Tempo atualizado da última verificação
             */
            if ((current_time - last_check_time) >= 5) {
                set_background_block(base_block_address, 1, 4, 6);
                base_block_address -= 1;
                last_check_time = time(NULL); /*Atualiza o tempo da última verificação*/
            } /*cada 5 seg*/

          
            /**
             * \brief Encerra o jogo após 60 segundos de execução efetiva
             * 
             * Quando o tempo total de jogo, descontando o tempo de pausa, atinge 60 segundos, 
             * a função define o estado para ENDED_BY_TIME, limpa os sprites e o bloco de fundo, e 
             * desenha a tela de "Game Over".
             * \param[in]       current_time: Tempo atual
             * \param[in]       start_time: Tempo de início do jogo
             * \param[in]       total_pause_time: Tempo total de pausa
             * \param[out]      state: Novo estado do jogo após a ação
             */
            if ((current_time - start_time - total_pause_time) >= 60) {
                state = ENDED_BY_TIME;
                clear_sprite();
                clear_background_block();
                draw_game_over_screen();
            }
        }
    }
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

    fd = open(MOUSEFILE, O_RDONLY); // Abre dispositivo do mouse
    if (fd == -1) {
        perror("Não é possível abrir o dispositivo do mouse");
        exit(EXIT_FAILURE);
    }

    while(1) {

        if (state == ENDED_BY_BUTTON) {
            break;
        }

        if (read(fd, &mouse_buffer, sizeof(mouse_buffer)) > 0) {

            x_disp = mouse_buffer[1];
            y_disp = mouse_buffer[2];
            leftButton = mouse_buffer[0] & 0x1;

            x += x_disp;
            y -= y_disp;

            limitarCursor(&x, &y);


            set_sprite(martelo->data_register, x, y, martelo->offset, 1);
            change_coordinate(martelo, x, y);


            /**
             * \brief Verifica colisões e atualiza a pontuação
             * 
             * A função verifica se o martelo colidiu com alguma toupeira enquanto o botão esquerdo 
             * do mouse está pressionado e a toupeira está dentro dos limites. Se houver colisão, 
             * a pontuação é incrementada.
             * \param[in]       toupeiras: Array de toupeiras
             * \param[in]       martelo: Sprite do martelo
             * \param[in]       leftButton: Estado do botão esquerdo do mouse
             * \param[in]       state: Estado atual do jogo
             * \param[out]      pontuacao: Pontuação atualizada
             */
            int i;
            for (i = 0; i < 9; i++) {
                 if (collision(toupeiras[i], martelo) && leftButton && toupeiras[i]->coord_y <= toupeiras[i]->max_y && state == RUNNING) {
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

    }


    return NULL;
}

/* Escreve sprites na memória a partir do offset 25 */
void write_sprites() {

    /**
     * \brief Escreve os sprites do martelo na memória
     * 
     * A função escreve os dados dos sprites do martelo na memória, começando no endereço 10000.
     * \param[in]       marteloSp: Dados dos RGB sprites do martelo
     * \param[out]      endereco_memoria: Endereço na memória onde os dados são escritos
     */
    int i;
    for (i = 0; i < 380; i++) {
        int R = marteloSp[i][0];
        int G = marteloSp[i][1];
        int B = marteloSp[i][2];
        int endereco_memoria = 10000 + i;
        write_sprite_mem(R, G, B, endereco_memoria);
    }

    /**
     * \brief Escreve os sprites das toupeiras na memória
     * 
     * A função escreve os dados dos sprites das toupeiras na memória, começando no endereço 10400.
     * \param[in]       toupeiraSp: Dados dos RGB sprites das toupeiras
     * \param[out]      endereco_memoria: Endereço na memória onde os dados são escritos
     */
    for (i = 0; i < 400; i++) {
        int R = toupeiraSp[i][0];
        int G = toupeiraSp[i][1];
        int B = toupeiraSp[i][2];
        int endereco_memoria = 10400 + i;
        write_sprite_mem(R, G, B, endereco_memoria);
    }

    /**
     * \brief Escreve os sprites dos arbustos na memória
     * 
     * A função escreve os dados dos sprites dos arbustos na memória, começando no endereço 10800.
     * \param[in]       arbustoSp: Dados dos RGB sprites dos arbustos
     * \param[out]      endereco_memoria: Endereço na memória onde os dados são escritos
     */
    for (i = 0; i < 400; i++) {
        int R = arbustoSp[i][0];
        int G = arbustoSp[i][1];
        int B = arbustoSp[i][2];
        int endereco_memoria = 10800 + i;
        write_sprite_mem(R, G, B, endereco_memoria);
    }
}


int main() {
    
    pthread_t thread1, thread2;

    mapPeripherals();
    clear_sprite();
    draw_initial_screen();

    write_sprites();

    /* Apaga os 3 primeiros dígitos do mostrador de 7 segmentos */
    *HEX3_ptr = 0b1111111;
    *HEX4_ptr = 0b1111111;
    *HEX5_ptr = 0b1111111;
    


/* ============ CRIAÇÃO DE SPRITES ============= */


    /**
     * \brief Inicializa as toupeiras
     * 
     * Cria e configura as instâncias das toupeiras para a primeira linha com as propriedades
     * de coordenadas, offset, data register e limites de movimento.
     */

    /* Toupeiras - linha 1*/
    Sprite toupeira1;
    toupeira1.coord_x = 200;
    toupeira1.coord_y = 300;
    toupeira1.offset = 26;
    toupeira1.data_register = 11;
    toupeira1.ativo = 1;
    toupeira1.direction = 1;
    toupeira1.moving = 1;
    toupeira1.min_y = 300;
    toupeira1.max_y = 285;

    Sprite toupeira2;
    toupeira2.coord_x = 300;
    toupeira2.coord_y = 300;
    toupeira2.offset = 26;
    toupeira2.data_register = 12;
    toupeira2.ativo = 1;
    toupeira2.direction = 1;
    toupeira2.moving = 1;
    toupeira2.min_y = 300;
    toupeira2.max_y = 285;

    Sprite toupeira3;
    toupeira3.coord_x = 400;
    toupeira3.coord_y = 300;
    toupeira3.offset = 26;
    toupeira3.data_register = 13;
    toupeira3.ativo = 1;
    toupeira3.direction = 1;
    toupeira3.moving = 1;
    toupeira3.min_y = 300;
    toupeira3.max_y = 285;

    /**
     * \brief Inicializa as toupeiras para a segunda linha
     * 
     * Cria e configura as instâncias das toupeiras para a segunda linha com as propriedades
     * de coordenadas, offset, data register e limites de movimento.
     */

    /* Toupeiras - linha 2 */
    Sprite toupeira4;
    toupeira4.coord_x = 200;
    toupeira4.coord_y = 350;
    toupeira4.offset = 26;
    toupeira4.data_register = 14;
    toupeira4.ativo = 1;
    toupeira4.direction = 1;
    toupeira4.moving = 1;
    toupeira4.min_y = 350;
    toupeira4.max_y = 335;

    Sprite toupeira5;
    toupeira5.coord_x = 300;
    toupeira5.coord_y = 350;
    toupeira5.offset = 26;
    toupeira5.data_register = 15;
    toupeira5.ativo = 1;
    toupeira5.direction = 1;
    toupeira5.moving = 1;
    toupeira5.min_y = 350;
    toupeira5.max_y = 335;

    Sprite toupeira6;
    toupeira6.coord_x = 400;
    toupeira6.coord_y = 350;
    toupeira6.offset = 26;
    toupeira6.data_register = 16;
    toupeira6.ativo = 1;
    toupeira6.direction = 1;
    toupeira6.moving = 1;
    toupeira6.min_y = 350;
    toupeira6.max_y = 335;


    /**
     * \brief Inicializa as toupeiras para a terceira linha
     * 
     * Cria e configura as instâncias das toupeiras para a terceira linha com as propriedades
     * de coordenadas, offset, data register e limites de movimento.
     */
    /* Toupeiras - linha 3 */
    Sprite toupeira7;
    toupeira7.coord_x = 200;
    toupeira7.coord_y = 400;
    toupeira7.offset = 26;
    toupeira7.data_register = 17;
    toupeira7.ativo = 1;
    toupeira7.direction = 1;
    toupeira7.moving = 1;
    toupeira7.min_y = 400;
    toupeira7.max_y = 385;

    Sprite toupeira8;
    toupeira8.coord_x = 300;
    toupeira8.coord_y = 400;
    toupeira8.offset = 26;
    toupeira8.data_register = 18;
    toupeira8.ativo = 1;
    toupeira8.direction = 1;
    toupeira8.moving = 1;
    toupeira8.min_y = 400;
    toupeira8.max_y = 385;

    Sprite toupeira9;
    toupeira9.coord_x = 400;
    toupeira9.coord_y = 400;
    toupeira9.offset = 26;
    toupeira9.data_register = 19;
    toupeira9.ativo = 1;
    toupeira9.direction = 1;
    toupeira9.moving = 1;
    toupeira9.min_y = 400;
    toupeira9.max_y = 385;

    /**
     * \brief Inicializa as toupeiras e as armazena em um array
     * 
     * Cria um array de ponteiros para as instâncias das toupeiras, permitindo acesso
     * para manipulação durante a execução do jogo.
     */
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

    /**
     * \brief Inicializa o martelo
     * 
     * Cria e configura a instância do martelo com as propriedades de coordenadas, offset,
     * data register e estado de atividade.
     */
    Sprite_Fixed martelo;
    martelo.coord_x = 0;
    martelo.coord_y = 0;
    martelo.offset = 25;
    martelo.ativo = 1;
    martelo.data_register = 1;


    /**
     * \brief Inicializa os arbustos
     * 
     * Cria e configura as instâncias dos arbustos para cada linha com as propriedades
     * de coordenadas, offset, data register e estado de atividade.
     */
        /* Arbustos - linha 1 */
    Sprite_Fixed arbusto1;
    arbusto1.coord_x = 200;
    arbusto1.coord_y = 300;
    arbusto1.offset = 27;
    arbusto1.ativo = 1;
    arbusto1.data_register = 2; 

    Sprite_Fixed arbusto2;
    arbusto2.coord_x = 300;
    arbusto2.coord_y = 300;
    arbusto2.offset = 27;
    arbusto2.ativo = 1;
    arbusto2.data_register = 3; 

    Sprite_Fixed arbusto3;
    arbusto3.coord_x = 400;
    arbusto3.coord_y = 300;
    arbusto3.offset = 27;
    arbusto3.ativo = 1;
    arbusto3.data_register = 4; 


    /**
     * \brief Inicializa os arbustos para a segunda linha
     * 
     * Cria e configura as instâncias dos arbustos para a segunda linha com as propriedades
     * de coordenadas, offset, data register e estado de atividade.
     */
    /* Arbustos - linha 2 */
    Sprite_Fixed arbusto4;
    arbusto4.coord_x = 200;
    arbusto4.coord_y = 350;
    arbusto4.offset = 27;
    arbusto4.ativo = 1;
    arbusto4.data_register = 5; 

    Sprite_Fixed arbusto5;
    arbusto5.coord_x =  300;
    arbusto5.coord_y = 350;
    arbusto5.offset = 27;
    arbusto5.ativo = 1;
    arbusto5.data_register = 6; 

    Sprite_Fixed arbusto6;
    arbusto6.coord_x = 400;
    arbusto6.coord_y = 350;
    arbusto6.offset = 27;
    arbusto6.ativo = 1;
    arbusto6.data_register = 7; 

    /**
     * \brief Inicializa os arbustos para a terceira linha
     * 
     * Cria e configura as instâncias dos arbustos para a terceira linha com as propriedades
     * de coordenadas, offset, data register e estado de atividade.
     */
    /* Arbustos - linha 3 */
    Sprite_Fixed arbusto7;
    arbusto7.coord_x = 200;
    arbusto7.coord_y = 400;
    arbusto7.offset = 27;
    arbusto7.ativo = 1;
    arbusto7.data_register = 8; 

    Sprite_Fixed arbusto8;
    arbusto8.coord_x = 300;
    arbusto8.coord_y = 400;
    arbusto8.offset = 27;
    arbusto8.ativo = 1;
    arbusto8.data_register = 9; 

    Sprite_Fixed arbusto9;
    arbusto9.coord_x = 400;
    arbusto9.coord_y = 400;
    arbusto9.offset = 27;
    arbusto9.ativo = 1;
    arbusto9.data_register = 10; 

    /**
     * \brief Inicializa os arbustos e os armazena em um array
     * 
     * Cria um array de ponteiros para as instâncias dos arbustos, permitindo acesso
     * fácil para manipulação durante a execução do jogo.
     */
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

    /**
     * \brief Prepara os argumentos para as threads
     * 
     * Cria um array de ponteiros para passar as referências do martelo, das toupeiras e dos arbustos
     * para as threads que serão criadas.
     */
    void* args[3] = { &martelo, &toupeiras, &arbustos};

  
    /*============== CRIAÇÃO DE THREADS ================== */
    /**
     * \brief Cria e inicia as threads para movimento das toupeiras e controle do mouse
     * 
     * Cria duas threads: uma para movimentar as toupeiras e outra para o controle do mouse.
     * As threads são configuradas e os erros de criação e junção são tratados.
     */
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

    return 0;
}

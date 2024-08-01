#include "headers/address_map_arm.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>

// Ponteiros voláteis para os periféricos
volatile int* KEY_ptr;
volatile int* HEX0_ptr; 
volatile int* HEX1_ptr;
volatile int* HEX2_ptr;
volatile int* HEX3_ptr;
volatile int* HEX4_ptr;
volatile int* HEX5_ptr;


/* Array para mapeamento dos segmentos em binário para decimal */
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

int button0, button1, button2, button3;

/**
 * \brief           Mapeia os periféricos para endereços virtuais
 * \note            Esta função abre o arquivo /dev/mem e mapeia a ponte de luz 
 *                  para endereços virtuais, permitindo acesso aos periféricos 
 *                  conectados. Os ponteiros para os periféricos são inicializados
 *                  com os endereços mapeados.
 */
void mapPeripherals() {
    
    int fd = -1; // usado para abrir /dev/mem
    void *LW_virtual; // Endereço virtual para a ponte


    // Abre /dev/mem para dar acesso aos endereços físicos
    if ((fd = open("/dev/mem", (O_RDWR | O_SYNC))) == -1) {
        printf("ERROR: could not open \"/dev/mem\"\n");
        return;
    }

    // Obtém um mapeamento de endereços físicos para endereços virtuais
    LW_virtual = mmap(NULL, LW_BRIDGE_SPAN, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, LW_BRIDGE_BASE);
    if (LW_virtual == MAP_FAILED) {
        printf("ERROR: mmap() failed\n");
        close(fd);
        return;
    }

    // Define ponteiros para os endereços I/O (LEDR_BASE é assumido como definido)
    KEY_ptr = (int *)(LW_virtual + KEYS_BASE);
    HEX0_ptr = (int *)(LW_virtual + HEX0_BASE);
    HEX1_ptr = (int *)(LW_virtual + HEX1_BASE);
    HEX2_ptr = (int *)(LW_virtual + HEX2_BASE);
    HEX3_ptr = (int *)(LW_virtual + HEX3_BASE);
    HEX4_ptr = (int *)(LW_virtual + HEX4_BASE);
    HEX5_ptr = (int *)(LW_virtual + HEX5_BASE);

}

/**
 * \brief           Limita a posição do cursor dentro dos limites especificados
 * \param[in,out]   x: Posição horizontal do cursor a ser limitada
 * \param[in,out]   y: Posição vertical do cursor a ser limitada
 * \note            Esta função ajusta a posição do cursor para garantir que ele
 *                  permaneça dentro dos limites de 1 a 620 para x e de 1 a 461
 *                  para y.
 */
void limitarCursor(int *x, int *y) {
    if (*x <= 1) *x = 1;
    if (*y <= 1) *y = 1;
    if (*x >= 620) *x = 620;
    if (*y >= 461) *y = 461;
}

/**
 * \param[in]       number: Número a ser convertido e exibido
 * \return          Valor binário correspondente ao número que será exibido nos segmentos
 * \note            Esta função retorna a representação binária do número fornecido
 *                  para ser exibida nos segmentos.
 */
uint8_t display(int number) {
    return segmentos[number];
}

/**
 * \brief Função que lê o estado dos botões e atualiza as variáveis correspondentes
 * 
 * Esta função lê os valores dos botões conectados ao dispositivo e 
 * atualiza as variáveis button0, button1, button2 e button3 com base 
 * no estado de cada botão. Cada botão é mapeado para um bit específico
 * na variável KEY_ptr.
 */
void readButtons() {
    button0 = ((*KEY_ptr & 0b0001) == 0); // iniciar
    button1 = ((*KEY_ptr & 0b0010) == 0); // pausar
    button2 = ((*KEY_ptr & 0b0100) == 0); // reiniciar
    button3 = ((*KEY_ptr & 0b1000) == 0); // parar
}
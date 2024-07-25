#include <asm/io.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/uaccess.h>
// #include <stdint.h>
#include "headers/address_map_arm.h"

#define MAX_SIZE     32
#define BASE_MINOR   71
#define DEVICE_COUNT 1
#define DEVICE_NAME  "graphicProcessor"

#define OPCODE_WBR   0b0000
#define OPCODE_WBM   0b0010
#define OPCODE_DP    0b0011
#define OPCODE_WSM   0b0001
#define WBR          1
#define WBM          2
#define DP           3
#define WSM          4

void* LW_virtual;
volatile int* data_a_ptr; /*Ponteiro para registrador de dados A */
volatile int* data_b_ptr; /*Ponteiro para registrador de dados B */
volatile int* wrreg_ptr; /* Ponteiro para registrador de escrita */
volatile int* wrfull_ptr; /* Ponteiro para registrador de fila cheia */

static char msg[MAX_SIZE]; /*Buffer de mensagem para comunicação com o usuário */
static dev_t device_number = 0; /*Número do dispositivo alocado dinamicamente */
static struct cdev cdev; /*Estrutura do dispositivo de caractere */
static struct class* class = NULL; /* Estrutura da classe do dispositivo */

/*Protótipos das funções do módulo*/
static int __init iniciar(void);
static void __exit parar(void);
static int device_open(struct inode* inode, struct file* filp);
static int device_release(struct inode* inode, struct file* filp);
static ssize_t device_read(struct file* filp, char __user* buf, size_t len, loff_t* off);
static ssize_t device_write(struct file* filp, const char __user* buf, size_t len, loff_t* off);
static void escrita_buffer(void);
static int instruction_DP(int forma, int R, int G, int B, int tamanho, int x, int y, int endereco);
static int instruction_WBR(int R, int G, int B, int reg, int x, int y, int offset, int sp);
static int instruction_WBM(int endereco_memoria, int R, int G, int B);
static int instruction_WSM(int R, int G, int B, int endereco_memoria);

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = device_open,
    .read = device_read,
    .write = device_write,
    .release = device_release,
};

/**
 * \brief Função de inicialização do módulo de kernel
 * 
 * Esta função é chamada quando o módulo do kernel é carregado. Ela inicializa
 * o número do dispositivo, configura a estrutura do dispositivo de caractere,
 * cria a classe do dispositivo, mapeia os endereços de I/O, inicializa os 
 * ponteiros para registradores e imprime uma mensagem de inicialização
 * 
 * \return 0 se a inicialização foi bem sucedida
 */
static int __init iniciar(void) {
    int err = 0;

    if ((err = alloc_chrdev_region(&device_number, BASE_MINOR, DEVICE_COUNT, DEVICE_NAME)) < 0) { /*Aloca o número de dispositivo de caractere*/
       // printk(KERN_ERR "alloc_chrdev_region() falhou, erro %d\n", err);
        return err;
    }

    
    cdev_init(&cdev, &fops); /*Inicializa a estrutura de dispositivo de caractere*/
    cdev.owner = THIS_MODULE;

    
    if ((err = cdev_add(&cdev, device_number, BASE_MINOR)) < 0) { /* Adiciona o dispositivo de caractere ao sistema*/
       // printk(KERN_ERR "cdev_add() falhou, erro: %d\n", err);
        return err;
    }

    /*Criação da classe de dispositivo*/
    class = class_create(THIS_MODULE, DEVICE_NAME); 
    device_create(class, NULL, device_number, NULL, DEVICE_NAME);

    
    LW_virtual = ioremap_nocache(LW_BRIDGE_BASE, LW_BRIDGE_SPAN);/*Mapeamento de memória entrada e saida I/O*/

    /*Inicialização dos ponteiros para registradores*/
    data_a_ptr = (int*)(LW_virtual + DATA_A_BASE);
    data_b_ptr = (int*)(LW_virtual + DATA_B_BASE);
    wrreg_ptr = (int*)(LW_virtual + WRREG_BASE);
    wrfull_ptr = (int*)(LW_virtual + WRFULL);

   // printk(KERN_INFO "Driver carregado no sistema\n");

    return 0;
}

/**
 * \brief Função de saída do módulo de kernel.
 * 
 * Esta função é chamada quando o módulo do kernel é descarregado. Ela remove o
 * dispositivo de caractere, destroi a classe do dispositivo, libera o número
 * de dispositivo alocado, desmapeia o endereço de I/O e imprime uma mensagem
 * de remoção
 */
static void __exit parar(void) {
    device_destroy(class, device_number);
    class_destroy(class);
    cdev_del(&cdev);
    unregister_chrdev_region(device_number, DEVICE_COUNT);
    iounmap(LW_virtual);

   // printk(KERN_INFO "Driver removido do sistema\n");
}

/**
 * \brief Função chamada quando o dispositivo de caractere é aberto
 * 
 * \param[in] inode : Estrutura de inode do dispositivo
 * \param[in] file : Estrutura de arquivo do dispositivo
 * \return 0 se a abertura foi bem sucedida
 */
static int device_open(struct inode* inode, struct file* file) {
    //printk(KERN_INFO "Arquivo aberto no espaço do usuário\n");
    return 0;
}

/**
 * \brief Função chamada quando o dispositivo de caractere é fechado
 * 
 * \param[in] inode : Estrutura de inode do dispositivo
 * \param[in] file : Estrutura de arquivo do dispositivo
 * \return 0 quando o arquivo é fechado
 */
static int device_release(struct inode* inode, struct file* file) {
    //printk(KERN_INFO "Arquivo fechado\n");
    return 0;
}

/**
 * \brief Função para leitura de dados do dispositivo de caractere
 * 
 * \param[in] filp : Estrutura de arquivo do dispositivo
 * \param[in] buffer : Buffer do usuário para armazenar os dados lidos
 * \param[in] length : Tamanho dos dados a serem lidos
 * \param[in] offset : Posição de leitura no arquivo
 * \return Número de bytes lidos ou erro
 */
static ssize_t device_read(struct file* filp, char* buffer, size_t length, loff_t* offset) {

    /*Copia a mensagem para o buffer do usuário*/
    if (copy_to_user(buffer, msg, length) != 0) {
        return -EFAULT;
    }
    return length;
}

/**
 * \brief Função para escrita de dados no dispositivo de caractere
 * 
 * \param[in] filp : Estrutura de arquivo do dispositivo
 * \param[in] buffer : Buffer do usuário contendo os dados a serem escritos
 * \param[in] length : Tamanho dos dados a serem escritos
 * \param[in] offset : Posição de escrita no arquivo
 * \return Número de bytes escritos ou erro
 */
static ssize_t device_write(struct file* filp, const char* buffer, size_t length, loff_t* offset) {

    int values[MAX_SIZE];
    int instruction = 0;

    while (*wrfull_ptr) {} /*Aguarda a fila esvaziar antes de mandar novas instruções*/

    /*Copia os dados do buffer do usuário para a mensagem*/
    if (copy_from_user(msg, buffer, length) != 0) {
        return -EFAULT;
    }

    /*Lê a instrução da mensagem*/
    sscanf(msg, "%d", &values[0]);
    instruction = values[0];

    /*Executa a instrução com base no código*/
    switch (instruction) {
        case WBR:
            sscanf(msg, "%d %d %d %d %d %d %d %d %d", &values[0], &values[1], &values[2], &values[3], &values[4],
                &values[5], &values[6], &values[7], &values[8]);
            instruction_WBR(values[1], values[2], values[3], values[4], values[5], values[6], values[7], values[8]);
            break;
        case WBM:
            sscanf(msg, "%d %d %d %d %d", &values[0], &values[1], &values[2], &values[3], &values[4]);
            instruction_WBM(values[1], values[2], values[3], values[4]);
            break;
        case DP:
            sscanf(msg, "%d %d %d %d %d %d %d %d %d", &values[0], &values[1], &values[2], &values[3], &values[4],
                &values[5], &values[6], &values[7], &values[8]);
            instruction_DP(values[1], values[2], values[3], values[4], values[5], values[6], values[7], values[8]);
            break;
        case WSM:
            sscanf(msg, "%d %d %d %d %d", &values[0], &values[1], &values[2], &values[3], &values[4]);
            instruction_WSM(values[1], values[2], values[3], values[4]);
            break;
        default:
            break;
    }

    return length;
}

/**
 * \brief Função para escrita no buffer
 * 
 * Esta função envia um sinal de escrita ao processador gráfico,
 * indicando que os dados estão prontos para serem processados
 */
static void escrita_buffer(void) {
    *wrreg_ptr = 1;
    *wrreg_ptr = 0;
}

/**
 * \brief Instrução para escrita de background e sprite
 * 
 * \param[in] R : Componente vermelho
 * \param[in] G : Componente verde
 * \param[in] B : Componente azul
 * \param[in] reg : Número do registrador
 * \param[in] x : Posição X do sprite
 * \param[in] y : Posição Y do sprite
 * \param[in] offset : Deslocamento
 * \param[in] sp : Se não nulo, sprite a ser desenhado
 * \return 1 se a operação foi bem sucedida
 */
static int instruction_WBR(int R, int G, int B, int reg, int x, int y, int offset, int sp) {

    uint32_t data_b_value;
    
    uint32_t data_a_value = ((reg & 0x1F) << 4) | OPCODE_WBR;
    //*data_a_ptr = ((reg & 0x1F) << 4) | OPCODE_WBR;
    if (sp) {
        data_b_value = ((sp & 0x1) << 29) | ((x & 0x3FF) << 19) | ((y & 0x3FF)<< 9) | (offset & 0x1FF);
    } else {
        data_b_value = ((B & 0x7) << 6) | ((G & 0x7) << 3) | (R & 0x7);
    }

    *data_a_ptr = data_a_value;
    *data_b_ptr = data_b_value;

    escrita_buffer();
    return 1;
}

/**
 * \brief Instrução para escrita na memória de background
 * 
 * \param[in] endereco_memoria : Endereço na memória
 * \param[in] R : Componente vermelho
 * \param[in] G : Componente verde
 * \param[in] B : Componente azul
 * \return 1 se a operação foi bem-sucedida
 */
// static int instruction_WBM(int endereco_memoria, int R, int G, int B) {
//     // Limitar R, G e B a 3 bits
//     R &= 0x7;
//     G &= 0x7;
//     B &= 0x7;
//     endereco_memoria &= 0x1FFF; // 13 bits
    
//     *data_a_ptr = (endereco_memoria << 4) | OPCODE_WBM;
//     *data_b_ptr = (B << 6) | (G << 3) | R;
    
//     escrita_buffer();
//     return 1;
// }

static int instruction_WBM(int endereco_memoria, int R, int G, int B) {
    // Limitar R, G e B a 3 bits
    
    // Limitar endereco_memoria a 13 bits
    
    uint32_t data_a_value = ((endereco_memoria & 0x1FFF) << 4) | OPCODE_WBM;
    uint32_t data_b_value = ((B & 0x7) << 6) | ((G & 0x7) << 3) | (R & 0x7);

    // printf("Data A: 0x%X\n", data_a_value);
    // printf("Data B: 0x%X\n", data_b_value);


    *data_a_ptr = data_a_value;
    *data_b_ptr = data_b_value;
    
    // Chamar a função de escrita no buffer
    escrita_buffer();

    return 1;
}


/**
 * \brief Instrução para desenho de poligono no processador gráfico
 * 
 * \param[in] forma : Forma do poligono
 * \param[in] R : Componente vermelho
 * \param[in] G : Componente verde
 * \param[in] B : Componente azul
 * \param[in] tamanho : Tamanho do poligono
 * \param[in] x : Posição X do poligono
 * \param[in] y : Posição Y do poligono
 * \param[in] endereco : Endereço de memoria
 * \return 1 se a operação foi bemsucedida
 */
static int instruction_DP(int forma, int R, int G, int B, int tamanho, int x, int y, int endereco) {
    // Limitar R, G e B a 3 bits
    R &= 0x7;
    G &= 0x7;
    B &= 0x7;

    *data_a_ptr = (endereco << 4) | OPCODE_DP;
    *data_b_ptr = (forma << 31) | (B << 28) | (G << 25) | (R << 22) | (tamanho << 18) | (y << 9) | x;
    escrita_buffer();
    return 1;
}

/**
 * \brief Instrução para escrita de sprite na memória
 * 
 * \param[in] R : Componente vermelho
 * \param[in] G : Componente verde
 * \param[in] B : Componente azul
 * \param[in] endereco_memoria : Endereço na memória
 * \return 1 se a operação foi bem sucedida
 */
static int instruction_WSM(int R, int G, int B, int endereco_memoria) {
    // Limitar R, G e B a 3 bits
    R &= 0x7;
    G &= 0x7;
    B &= 0x7;

    *data_a_ptr = (endereco_memoria << 4) | OPCODE_WSM;
    *data_b_ptr = (B << 6) | (G << 3) | R;

    escrita_buffer();
    return 1;
}

MODULE_AUTHOR("TEC499-TP02-G02");
MODULE_DESCRIPTION("Driver de caracter pro processador gráfico");
MODULE_LICENSE("GPL");

/*Inicialização e saída do módulo*/
module_init(iniciar);
module_exit(parar);

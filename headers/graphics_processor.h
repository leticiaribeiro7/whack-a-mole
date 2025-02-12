/**
 * Este código foi adaptado de um documento PDF.
 * Fonte: Desenvolvimento de uma Arquitetura Baseada em Sprites para criação de Jogos 2D em Ambientes
 * Reconfiguráveis utilizando dispositivos FPGA, Gabriel B. Alves e Anfranserai M. Dias, Páginas 9-10.
 *
 * URL:  https://drive.google.com/file/d/1MlIlpB9TSnoPGEMkocr36EH9-CFz8psO/view
 */

// Exemplos de constantes para fixar valores de movimento de um sprite
#define LEFT         0
#define RIGHT        4
#define UP           2
#define DOWN         6
#define UPPER_RIGHT  1
#define UPPER_LEFT   3
#define BOTTOM_LEFT  5
#define BOTTOM_RIGHT 7

/**
 * Estrutura que define um sprite móvel.
 * 
 * Atributos coord_x e coord_y: armazenam as coordenadas x e y do sprite. 
 * Atributo direction: armazena um número inteiro indicando o ângulo de movimento. 
 * Atriburo offset: indica o deslocamento na memória utilizado para selecionar o bitmap
 * armazenado no Processador Gráfico.
 * Atributo data_register: indica o registrador do Banco de Registradores.
 * Atributos step_x e step_y: representam os passos nas direções x e y.
 * Atributo ativo: indica se o sprite está ativo ou não.
 * Atributo collision: indica se houve uma colisão com outro sprite.
*/
typedef struct {
    int coord_x, coord_y;
    int direction, offset, data_register;
    int ativo, collision, moving;
    int min_y, max_y, interval, last_update; 
} Sprite;

/**
 * Estrutura que define sprites fixos.
 * 
 * Não possuem movimentação durante o jogo.
 * Armazena coordenadas, deslocamentos na memória,
 * registrador de dados e estado do sprite.
 * */
typedef struct {
    int coord_x, coord_y, offset;
    int data_register, ativo;
} Sprite_Fixed;

/**
 * \brief Posicionar um sprite na tela
 * 
 * \param[in] registrador : Define o registrador
 * \param[in] x : Definição da coordenada x do sprite
 * \param[in] y : Definição da coordenada y do sprite
 * \param[in] offset : Deslocamento na memória para selecionar o bitmap
 * \param[in] activation_bit : Estado de ativação do sprite
 * \return  1 caso operação seja bem sucedida, e 0 caso contrário
*/
void set_sprite(uint8_t reg, uint16_t x, uint16_t y, uint16_t offset, uint8_t activation_bit);

/**
 * \brief Modelar o background através de preenchimento dos blocos.
 * 
 * \param[in] endereco_memoria : Endereço de memória correspondente ao bloco a ser preenchido
 * \param[in] R : Componente da cor vermelha.
 * \param[in] G : Componente da cor verde.
 * \param[in] B : Componente da cor azul.
 * \return          1 caso operação seja bem sucedida, e 0 caso contrário.
*/
void set_background_block(uint16_t endereco_memoria, uint8_t R, uint8_t G, uint8_t B);

/**
 * \brief Configura a cor base do background.
 * 
 * \param[in] R :Componente da cor vermelha.
 * \param[in] G : Componente da cor verde.
 * \param[in] B : Componente da cor azul.
 * \return    1 caso operação seja bem sucedida, e 0 caso contrário.
*/
void set_background_color(uint8_t R, uint8_t G, uint8_t B);

/**
 * \brief Posicionar um polígono na tela.
 * 
 * \param[in] forma : Formato do polígono, 1 para triângulo e 0 para quadrado.
 * \param[in] R : Componente da cor vermelha.
 * \param[in] G : Componente da cor verde.
 * \param[in] B : Componente da cor azul.
 * \param[in] tamanho : Tamanho do polígono, de 1 a 15.
 * \param[in] x : Coordenada x.
 * \param[in] y : Coordenada y.
 * \param[in] endereco : Endereço de memória onde o polígono será armazenado.
 * \return    1 caso operação seja bem sucedida, e 0 caso contrário.
*/
void define_poligon(uint8_t forma, uint8_t R, uint8_t G, uint8_t B, uint8_t tamanho, uint16_t x, uint16_t y, uint8_t endereco);

/**
 * \brief Define um novo sprite.
 * 
 * \param[in] R : Componente da cor vermelha.
 * \param[in] G : Componente da cor verde.
 * \param[in] B : Componente da cor azul.
 * \param[in] endereco_memoria : Endereço de memória onde o sprite será armazenado
 * \return    1 caso operação seja bem sucedida, e 0 caso contrário.
*/
void write_sprite_mem(uint8_t R, uint8_t G, uint8_t B, uint16_t endereco_memoria);

/**
 * \brief Responsável por atualizar as coordenadas x e y de um sprit móvel de acordo ao seu ângulo de movimento e valor de deslocamento.
 * 
 * \param[in] sp : Passagem por referência
 * \param[in] mirror : Coordenadas do sprite
*/
void change_coordinate(Sprite_Fixed* sp, int new_x, int new_y);

/**
 * \brief Verifica se ocorreu uma colisão entre dois sprites quaisquer.
 * 
 * \param[in] sp1 : Passagem por referência da coord_x.
 * \param[in] sp2 : Passagem por referência da coord_y.
 * \return      1 caso operação seja bem sucedida, e 0 caso contrário.
*/
int collision(Sprite* sp1, Sprite_Fixed* sp2);



void clear_background_color();
void clear_background_block();
void clear_poligonos();
void clear_sprite();
int write_to_bus(unsigned char *buffer);
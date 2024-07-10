#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include "./address_map_arm.h"

#define DEVICE_NAME "button_irq"
#define IRQ_NUM     73

void* LW_virtual;
volatile int* KEY_ptr;

/* Protótipos das funções do módulo */
static int __init iniciar(void);
static void __exit parar(void);
static irqreturn_t button_irq_handler(int irq, void* dev_id);

static int __init iniciar(void) {
    int err = 0;

    LW_virtual = ioremap_nocache(LW_BRIDGE_BASE, LW_BRIDGE_SPAN);
    if (!LW_virtual) {
        printk(KERN_ERR "Erro ao mapear memória\n");
        return -EIO;
    }

    /* Inicialização do ponteiro para registrador de botões */
    KEY_ptr = (int*)(LW_virtual + KEYS_BASE);

    err = request_irq(IRQ_NUM, button_irq_handler, IRQF_SHARED, DEVICE_NAME, (void*)(button_irq_handler));
    if (err) {
        printk(KERN_ERR "Erro ao solicitar IRQ: %d\n", err);
        iounmap(LW_virtual);
        return err;
    }

    printk(KERN_INFO "Módulo de interrupção de botões carregado\n");
    return 0;
}

/* Função de saída do módulo de kernel */
static void __exit parar(void) {
    free_irq(IRQ_NUM, (void*)(button_irq_handler));
    iounmap(LW_virtual);

    printk(KERN_INFO "Módulo de interrupção de botões descarregado\n");
}

/* Handler de interrupção dos botões */
static irqreturn_t button_irq_handler(int irq, void* dev_id) {
    int key_value;

    key_value = *KEY_ptr;

    *KEY_ptr = 0xF;

    /* Verificar qual botão foi pressionado */
    if (key_value & 0x1) {
        printk(KERN_INFO "Botão 1 pressionado\n");
    }
    if (key_value & 0x2) {
        printk(KERN_INFO "Botão 2 pressionado\n");
    }
    if (key_value & 0x4) {
        printk(KERN_INFO "Botão 3 pressionado\n");
    }
    if (key_value & 0x8) {
        printk(KERN_INFO "Botão 4 pressionado\n");
    }

    return IRQ_HANDLED;
}

MODULE_AUTHOR("TEC499-TP02-G02");
MODULE_DESCRIPTION("Driver de interrupção para botões");
MODULE_LICENSE("GPL");

module_init(iniciar);
module_exit(parar);

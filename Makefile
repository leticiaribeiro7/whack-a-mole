obj-m += graphics_processor_module.o
obj-m += buttons_interrupt.o

KDIR := /lib/modules/$(shell uname -r)/build

PWD := $(shell pwd)

# Alvo principal (default)
all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	sudo insmod graphics_processor_module.ko
	sudo insmod buttons_interrupt.ko

# Limpeza dos arquivos gerados pela compilação
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	sudo rmmod graphics_processor_module.ko
	sudo rmmod buttons_interrupt.ko

# Compilação da biblioteca e programa principal
lib:
	gcc -pthread graphics_processor_lib.c newMain.c -o main

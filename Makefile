obj-m += graphics_processor_module.o

KDIR := /lib/modules/$(shell uname -r)/build

PWD := $(shell pwd)

# Alvo principal (default)
all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	sudo insmod graphics_processor_module.ko

# Limpeza dos arquivos gerados pela compilação
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	sudo rmmod graphics_processor_module.ko

# Compilação da biblioteca e programa principal
lib:
	gcc -pthread graphics_processor_lib.c utils.c main.c -o main

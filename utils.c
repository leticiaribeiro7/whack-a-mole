#include "headers/address_map_arm.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>


volatile int* KEY_ptr;
volatile int* HEX0_ptr; 
volatile int* HEX1_ptr;
volatile int* HEX2_ptr;
volatile int* HEX3_ptr;
volatile int* HEX3_0_ptr;

void mapPeripherals() {
    
    int fd = -1; // used to open /dev/mem
    void *LW_virtual; // virtual address for light-weight bridge


    // Open /dev/mem to give access to physical addresses
    if ((fd = open("/dev/mem", (O_RDWR | O_SYNC))) == -1) {
        printf("ERROR: could not open \"/dev/mem\"\n");
        return;
    }

    // Get a mapping from physical addresses to virtual addresses
    LW_virtual = mmap(NULL, LW_BRIDGE_SPAN, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, LW_BRIDGE_BASE);
    if (LW_virtual == MAP_FAILED) {
        printf("ERROR: mmap() failed\n");
        close(fd);
        return;
    }

    // Set virtual address pointer to I/O port (LEDR_BASE is assumed to be defined)
    KEY_ptr = (int *)(LW_virtual + KEYS_BASE);
    HEX3_0_ptr = (int *)(LW_virtual + HEX3_HEX0_BASE);
    HEX0_ptr = (int *)(LW_virtual + HEX0_BASE);
    HEX1_ptr = (int *)(LW_virtual + HEX1_BASE);
    HEX2_ptr = (int *)(LW_virtual + HEX2_BASE);
    HEX3_ptr = (int *)(LW_virtual + HEX3_BASE);


}

void limitarCursor(int *x, int *y) {
    if (*x <= 1) *x = 1;
    if (*y <= 1) *y = 1;
    if (*x >= 620) *x = 620;
    if (*y >= 461) *y = 461;
}
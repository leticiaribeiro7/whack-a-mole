#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Função que será executada na thread
void* threadFunc1(void* arg) {
    while (1) {
        printf("Thread 1 is running\n");
        sleep(1); // Simula algum trabalho
    }
    return NULL;
}

void* threadFunc2(void* arg) {
    while (1) {
        printf("Thread 2 is running\n");
        sleep(1); // Simula algum trabalho
    }
    return NULL;
}

int main() {
    pthread_t thread1, thread2;

    // Cria as threads
    if (pthread_create(&thread1, NULL, threadFunc1, NULL) != 0) {
        perror("Failed to create thread 1");
        return 1;
    }
    if (pthread_create(&thread2, NULL, threadFunc2, NULL) != 0) {
        perror("Failed to create thread 2");
        return 1;
    }

    // Espera pelas threads (não é necessário neste caso específico, mas é uma boa prática)
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

#include <pthread.h>
#include <stdio.h>

#define HILOS 4
#define REPETICIONES 100000

int contador = 0;

void* incrementar(void* arg) {
    (void)arg;

    for (int i = 0; i < REPETICIONES; i++) {
        contador++;
    }

    return NULL;
}

int main(void) {
    pthread_t hilos[HILOS];

    for (int i = 0; i < HILOS; i++) {
        pthread_create(&hilos[i], NULL, incrementar, NULL);
    }

    for (int i = 0; i < HILOS; i++) {
        pthread_join(hilos[i], NULL);
    }

    printf("Esperado : %d\n", HILOS * REPETICIONES);
    printf("Obtenido : %d\n", contador);

    return 0;
}

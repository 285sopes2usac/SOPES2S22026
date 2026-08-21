#include <pthread.h>
#include <stdio.h>

#define HILOS 4
#define REPETICIONES 100000

int contador = 0;
pthread_mutex_t mutex;

void* incrementar(void* arg) {
    (void)arg;

    for (int i = 0; i < REPETICIONES; i++) {
        pthread_mutex_lock(&mutex);
        contador++;
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

int main(void) {
    pthread_t hilos[HILOS];

    pthread_mutex_init(&mutex, NULL);

    for (int i = 0; i < HILOS; i++) {
        pthread_create(&hilos[i], NULL, incrementar, NULL);
    }

    for (int i = 0; i < HILOS; i++) {
        pthread_join(hilos[i], NULL);
    }

    pthread_mutex_destroy(&mutex);

    printf("Esperado : %d\n", HILOS * REPETICIONES);
    printf("Obtenido : %d\n", contador);

    return 0;
}

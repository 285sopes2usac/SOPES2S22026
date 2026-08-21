#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

void* trabajador(void* arg) {
    int id = *(int*)arg;

    for (int i = 1; i <= 5; i++) {
        printf("Hilo %d -> Iteracion %d\n", id, i);
        sleep(1);
    }

    return NULL;
}

int main(void) {
    pthread_t hilos[3];
    int ids[3] = {1, 2, 3};

    for (int i = 0; i < 3; i++) {
        pthread_create(&hilos[i], NULL, trabajador, &ids[i]);
    }

    for (int i = 0; i < 3; i++) {
        pthread_join(hilos[i], NULL);
    }

    printf("\nTodos los hilos finalizaron.\n");
    return 0;
}

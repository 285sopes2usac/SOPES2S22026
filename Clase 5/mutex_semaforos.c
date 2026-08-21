#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define BUFFER 5
#define PRODUCTORES 2
#define CONSUMIDORES 2
#define TAREAS 10

int cola[BUFFER];
int entrada = 0;
int salida = 0;

pthread_mutex_t mutex;
sem_t espacios;
sem_t elementos;

void* productor(void* arg) {
    int id = *(int*)arg;

    for (int i = 0; i < TAREAS; i++) {
        int tarea = rand() % 100;

        sem_wait(&espacios);
        pthread_mutex_lock(&mutex);

        cola[entrada] = tarea;
        printf("Productor %d produjo tarea %d en posicion %d\n", id, tarea, entrada);
        entrada = (entrada + 1) % BUFFER;

        pthread_mutex_unlock(&mutex);
        sem_post(&elementos);

        usleep(150000);
    }

    return NULL;
}

void* consumidor(void* arg) {
    int id = *(int*)arg;

    for (int i = 0; i < (PRODUCTORES * TAREAS) / CONSUMIDORES; i++) {
        sem_wait(&elementos);
        pthread_mutex_lock(&mutex);

        int tarea = cola[salida];
        printf("Consumidor %d proceso tarea %d desde posicion %d\n", id, tarea, salida);
        salida = (salida + 1) % BUFFER;

        pthread_mutex_unlock(&mutex);
        sem_post(&espacios);

        usleep(200000);
    }

    return NULL;
}

int main(void) {
    pthread_t productores[PRODUCTORES];
    pthread_t consumidores[CONSUMIDORES];
    int ids_productores[PRODUCTORES];
    int ids_consumidores[CONSUMIDORES];

    srand((unsigned int)time(NULL));

    pthread_mutex_init(&mutex, NULL);
    sem_init(&espacios, 0, BUFFER);
    sem_init(&elementos, 0, 0);

    for (int i = 0; i < PRODUCTORES; i++) {
        ids_productores[i] = i + 1;
        pthread_create(&productores[i], NULL, productor, &ids_productores[i]);
    }

    for (int i = 0; i < CONSUMIDORES; i++) {
        ids_consumidores[i] = i + 1;
        pthread_create(&consumidores[i], NULL, consumidor, &ids_consumidores[i]);
    }

    for (int i = 0; i < PRODUCTORES; i++) {
        pthread_join(productores[i], NULL);
    }

    for (int i = 0; i < CONSUMIDORES; i++) {
        pthread_join(consumidores[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    sem_destroy(&espacios);
    sem_destroy(&elementos);

    printf("\nTodas las tareas fueron procesadas.\n");

    return 0;
}

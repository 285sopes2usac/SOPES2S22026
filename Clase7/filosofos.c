#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_FILOSOFOS 5

// Arreglo de mutexes: Cada tenedor es un recurso que requiere exclusión mutua
pthread_mutex_t tenedores[NUM_FILOSOFOS];

void* rutina_filosofo(void* arg) {
    int id = *(int*)arg;
    int tenedor_izq = id;
    int tenedor_der = (id + 1) % NUM_FILOSOFOS;

    printf("[Filosofo %d] Esta pensando profundamente...\n", id);
    sleep(1); // Pensando...

    // Romper la espera circular
    // Los filósofos pares toman primero el izquierdo, los impares el derecho
    if (id % 2 == 0) {
        pthread_mutex_lock(&tenedores[tenedor_izq]);
        pthread_mutex_lock(&tenedores[tenedor_der]);
    } else {
        pthread_mutex_lock(&tenedores[tenedor_der]);
        pthread_mutex_lock(&tenedores[tenedor_izq]);
    }

    // Sección crítica
    printf("[Filosofo %d] Tiene 2 tenedores y esta COMIENDO...\n", id);
    sleep(2); // Comiendo...

    // Salida de la sección crítica: libera los recursos para otros procesos
    pthread_mutex_unlock(&tenedores[tenedor_izq]);
    pthread_mutex_unlock(&tenedores[tenedor_der]);

    printf("[Filosofo %d] Termino de comer y libero los tenedores.\n", id);
    
    free(arg); // Liberar memoria dinámica del ID
    return NULL;
}

int main() {
    pthread_t filosofos[NUM_FILOSOFOS];

    printf("Iniciando la cena\n\n");

    // 1. Inicializar los mutexes (Tenedores en estado "desbloqueado")
    for (int i = 0; i < NUM_FILOSOFOS; i++) {
        pthread_mutex_init(&tenedores[i], NULL);
    }

    // 2. Crear los hilos (Filósofos)
    for (int i = 0; i < NUM_FILOSOFOS; i++) {
        int* id = malloc(sizeof(int));
        *id = i;
        pthread_create(&filosofos[i], NULL, rutina_filosofo, id);
    }

    // 3. Esperar a que terminen de comer
    for (int i = 0; i < NUM_FILOSOFOS; i++) {
        pthread_join(filosofos[i], NULL);
    }

    // 4. Limpiar los recursos
    for (int i = 0; i < NUM_FILOSOFOS; i++) {
        pthread_mutex_destroy(&tenedores[i]);
    }

    printf("\nTodos terminaron de cenar\n");
    return 0;
}
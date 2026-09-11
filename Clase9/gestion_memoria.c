#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * 1. GESTIÓN DE MEMORIA BÁSICA
 * 
 * Demuestra:
 * - Asignación dinámica (malloc, calloc, realloc)
 * - Liberación correcta de memoria
 * - Detección de fugas de memoria
 */

#define TAMAÑO_BLOQUE 1024

void ejemplo_malloc_free() {
    printf("=== MALLOC y FREE ===\n");
    
    // Asignar 1 KB de memoria
    int *array = (int *)malloc(TAMAÑO_BLOQUE * sizeof(int));
    if (array == NULL) {
        printf("Error: No hay memoria disponible\n");
        return;
    }
    
    printf("Asignadas %d bytes en 0x%p\n", TAMAÑO_BLOQUE * (int)sizeof(int), (void *)array);
    
    // Usar la memoria
    for (int i = 0; i < 10; i++) {
        array[i] = i * 2;
        printf("array[%d] = %d\n", i, array[i]);
    }
    
    // Liberar memoria
    free(array);
    array = NULL;  // Buena práctica: NULL después de free
    printf("Memoria liberada\n\n");
}

void ejemplo_calloc() {
    printf("=== CALLOC (inicializa a 0) ===\n");
    
    // CALLOC = asigna + inicializa a 0
    int *datos = (int *)calloc(10, sizeof(int));
    if (datos == NULL) {
        printf("Error: No hay memoria disponible\n");
        return;
    }
    
    printf("Datos asignados (ya están inicializados a 0):\n");
    for (int i = 0; i < 10; i++) {
        printf("datos[%d] = %d\n", i, datos[i]);
    }
    
    free(datos);
    printf("Memoria liberada\n\n");
}

void ejemplo_realloc() {
    printf("=== REALLOC (redimensiona memoria) ===\n");
    
    // Asignar 10 elementos
    int *buffer = (int *)malloc(10 * sizeof(int));
    printf("Buffer inicial: 10 elementos en 0x%p\n", (void *)buffer);
    
    for (int i = 0; i < 10; i++) {
        buffer[i] = i;
    }
    
    // Redimensionar a 20 elementos
    int *temp = (int *)realloc(buffer, 20 * sizeof(int));
    if (temp != NULL) {
        buffer = temp;
        printf("Buffer redimensionado a 20 elementos en 0x%p\n", (void *)buffer);
        
        // Llenar los nuevos elementos
        for (int i = 10; i < 20; i++) {
            buffer[i] = i * 10;
        }
        
        printf("Primeros 20 elementos:\n");
        for (int i = 0; i < 20; i++) {
            printf("%d ", buffer[i]);
        }
        printf("\n");
    }
    
    free(buffer);
    printf("Memoria liberada\n\n");
}

void ejemplo_fuga_memoria() {
    printf("=== DEMOSTRACIÓN: FUGA DE MEMORIA (INCORRECTO) ===\n");
    
    // ESTO ES UN ERROR - se pierde la referencia
    int *fuga = (int *)malloc(1000);
    printf("Asignada memoria en 0x%p\n", (void *)fuga);
    
    fuga = NULL;  // ¡PERDIMOS LA REFERENCIA! La memoria no se puede liberar
    printf("FUGA: No se puede liberar la memoria asignada\n");
    printf("La memoria sigue ocupada en el heap hasta que termina el programa\n\n");
}

int main(int argc, char *argv[]) {
    int habilitar_leak = 0; //

    if (argc > 1 && strcmp(argv[1], "leak") == 0) {
        habilitar_leak = 1;
    }

    ejemplo_malloc_free();
    ejemplo_calloc();
    ejemplo_realloc();

    if (habilitar_leak) {
        ejemplo_fuga_memoria();
    }

    return 0;
}
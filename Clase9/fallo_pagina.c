#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <string.h>

void print_page_faults(const char *mensaje) {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    // ru_minflt son fallos de página menores (las páginas se asignan en RAM)
    printf("%s - Fallos de pagina (Minor Page Faults): %ld\n", mensaje, usage.ru_minflt);
}

int main() {
    printf("=== DEMOSTRACION DE PAGINACION POR DEMANDA ===\n\n");
    print_page_faults("1. Al iniciar el programa");

    // Asignar 100 MB de memoria (Virtual, pero no física aún)
    long size = 100 * 1024 * 1024;
    char *memoria = (char *)malloc(size);
    
    if (memoria == NULL) {
        printf("Error al asignar memoria\n");
        return 1;
    }

    print_page_faults("2. Despues de malloc (Solo asignacion virtual)");

    // Forzar al OS a mapear la memoria a marcos físicos escribiendo en ella
    printf("\n[Escribiendo en la memoria para forzar Fallos de Pagina...]\n");
    // Escribimos en saltos de 4096 bytes (tamaño típico de una página en Linux)
    for (long i = 0; i < size; i += 4096) {
        memoria[i] = 'A'; 
    }

    print_page_faults("3. Despues de escribir en las paginas");

    free(memoria);
    return 0;
}
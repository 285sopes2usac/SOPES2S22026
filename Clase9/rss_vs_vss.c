#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Función para parsear el pseudo-archivo /proc/self/status
void print_memory_info(const char *step) { //step es un string que indica el paso actual del programa (ej. "Estado Inicial", "Después de malloc", etc.)
    char buffer[256];
    FILE *fp = fopen("/proc/self/status", "r"); // Abrimos el archivo de estado del proceso actual
    if (!fp) return;

    printf("\n--- %s ---\n", step);
    while (fgets(buffer, sizeof(buffer), fp)) {
        // Buscamos VmSize (VSS) y VmRSS (RSS)
        if (strncmp(buffer, "VmSize:", 7) == 0 || strncmp(buffer, "VmRSS:", 6) == 0) { // Imprimimos solo las líneas relevantes
            printf("%s", buffer);
        }
    }
    fclose(fp);
}

int main() {
    printf("PID del proceso actual: %d\n", getpid());
    
    print_memory_info("1. Estado Inicial");

    // Asignar 100 MB de memoria virtual (Aumentará el VSS, pero el RSS casi no cambia)
    size_t size = 100 * 1024 * 1024;
    char *mem = malloc(size); // malloc solo reserva espacio virtual, no asigna memoria física inmediatamente
    print_memory_info("2. Despues de malloc (El sistema solo asigna direcciones logicas)");

    // Escribir en la memoria para forzar al OS a mapear memoria física (Aumentará el RSS)
    printf("\n[Escribiendo datos en la memoria reservada...]\n");
    memset(mem, 1, size); // memset para escribir en toda la memoria asignada
    
    print_memory_info("3. Despues de escribir (Impacto real en RAM -> RSS aumenta)");

    free(mem);
    return 0;
}
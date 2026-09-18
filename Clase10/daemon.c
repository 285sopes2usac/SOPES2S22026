#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#define LOG_PATH "/tmp/my_daemon.log"

// Código base para el Daemon
void daemonizar() {
    pid_t pid;

    // Paso 1: Primer fork
    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE); // Fallo en el fork
    if (pid > 0) exit(EXIT_SUCCESS); // El padre termina

    // Paso 2: Nueva sesión y remoción de TTY
    if (setsid() < 0) exit(EXIT_FAILURE);

    // Paso 3 y 4: Entorno
    chdir("/");
    umask(0);

    // Paso 5: Redirección de descriptores
    int fd = open("/dev/null", O_RDWR);
    if (fd >= 0) {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        if (fd > 2) close(fd);
    }
}

static void registrar_evento(const char *mensaje) {
    FILE *log = fopen(LOG_PATH, "a");
    if (log == NULL) {
        return;
    }

    fprintf(log, "%s\n", mensaje);
    fclose(log);
}


// Manejo de señales y bucle

volatile sig_atomic_t ejecutar = 1; // sig_atomic_t es seguro para señales

void manejador_senales(int senal) {
    if (senal == SIGTERM) { // -15
        ejecutar = 0; // Rompe el bucle de manera segura
    }
    if (senal == SIGHUP) { // -1
        // Aquí podríamos recargar la configuración sin detener el daemon
        // Por ejemplo, leer un archivo de configuración actualizado
    }
}

int main() {
    // Inicializar el protocolo de daemonización
    daemonizar();

    char mensaje_inicio[128];
    snprintf(mensaje_inicio, sizeof(mensaje_inicio),
             "Daemon iniciado con PID %ld", (long)getpid());
    registrar_evento(mensaje_inicio);

    // Configurar la captura de señales de control
    signal(SIGTERM, manejador_senales);
    signal(SIGHUP, manejador_senales);

    // Bucle infinito de ejecución continua (Payload)
    while (ejecutar) {
        // Simulación de actividad de monitoreo
        registrar_evento("Daemon ejecutándose activamente en segundo plano...");
        sleep(5); // Pausa de 5 segundos para no saturar la CPU
    }

    registrar_evento("Daemon recibió SIGTERM y se está cerrando");

    return 0;
}
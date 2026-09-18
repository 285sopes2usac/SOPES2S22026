# Daemon

Un daemon es un proceso que se ejecuta en **segundo plano** de forma continua, sin necesidad de interacción directa del usuario. Ejemplos reales: servidores web (Apache, Nginx), gestores de correo (Postfix), bases de datos (MySQL), etc.

## Funcionalidad del Daemon

Este daemon realiza las siguientes acciones:

1. **Se daemoniza**: Se desvincula de la terminal que lo lanzó
2. **Registra su PID**: Anota su identificador único al arrancar
3. **Ejecuta un bucle de trabajo**: Cada 5 segundos registra un evento de actividad
4. **Maneja señales**: Reacciona a `SIGTERM` para cerrarse de forma segura
5. **Deja un registro en /tmp/my_daemon.log**: Permite auditar su comportamiento

---

## Partes Principales del Código

### 1. Función `daemonizar()` (líneas 12-35)
```c
void daemonizar() {
    pid_t pid;
    
    // Paso 1: Primer fork
    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);  // El padre muere
    
    // Paso 2: Nueva sesión (desvinculación de TTY)
    if (setsid() < 0) exit(EXIT_FAILURE);
    
    // Paso 3-4: Cambiar directorio y umask
    chdir("/");
    umask(0);
    
    // Paso 5: Redirigir stdin, stdout, stderr a /dev/null
    int fd = open("/dev/null", O_RDWR);
    if (fd >= 0) {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        if (fd > 2) close(fd);
    }
}
```
**Propósito**: Convierte el proceso en un daemon verdadero desvinculándolo de la terminal.

### 2. Función `registrar_evento()` (líneas 37-45)
```c
static void registrar_evento(const char *mensaje) {
    FILE *log = fopen(LOG_PATH, "a");
    if (log == NULL) {
        return;
    }
    
    fprintf(log, "%s\n", mensaje);
    fclose(log);
}
```
**Propósito**: Escribe eventos en el archivo de log `/tmp/my_daemon.log` para auditar el comportamiento.

### 3. Manejador de Señales (líneas 52-60)
```c
void manejador_senales(int senal) {
    if (senal == SIGTERM) {  // señal -15
        ejecutar = 0;  // Sale del bucle de forma segura
    }
}
```
**Propósito**: Permite que el daemon se cierre elegantemente cuando recibe `SIGTERM`.

### 4. Bucle Principal (líneas 62-85)
```c
int main() {
    daemonizar();
    
    // Registra el PID al arrancar
    snprintf(mensaje_inicio, sizeof(mensaje_inicio),
             "Daemon iniciado con PID %ld", (long)getpid());
    registrar_evento(mensaje_inicio);
    
    // Configura manejo de señales
    signal(SIGTERM, manejador_senales);
    signal(SIGHUP, manejador_senales);
    
    // Bucle de trabajo continuo
    while (ejecutar) {
        registrar_evento("Daemon ejecutándose...");
        sleep(5);  // Pausa de 5 segundos
    }
    
    // Registra el cierre
    registrar_evento("Daemon recibió SIGTERM y se está cerrando");
    
    return 0;
}
```
**Propósito**: Orquesta todo el ciclo de vida del daemon.

---

## Comandos Útiles

### Compilar el daemon
```bash
cd Clase10
gcc -Wall -Wextra -O2 daemon.c -o daemon
```

### Ejecutar el daemon
```bash
./daemon
```
El proceso se ejecuta en segundo plano automáticamente.

### Ver si el daemon sigue corriendo
```bash
pgrep -af daemon
```

### Ver el archivo de log
```bash
cat /tmp/my_daemon.log
```
Muestra toda la actividad registrada del daemon.

### Monitorear el log en tiempo real
```bash
tail -f /tmp/my_daemon.log
```
Se actualiza cada vez que el daemon escribe.

### Detener el daemon de forma segura (SIGTERM)
```bash
kill -TERM <PID>
# O más corto:
kill -15 <PID>
```

### Detener todos los daemons en ejecución
```bash
pkill -TERM -f daemon
```

### Ver el proceso en tiempo real con más detalles
```bash
ps aux | grep daemon
```

---

## Ciclo de Vida del Daemon

1. **Arranque**: `./daemon` → se daemoniza y registra su PID
2. **Ejecución**: Cada 5 segundos registra un evento en el log
3. **Parada**: `kill -TERM <PID>` → se cierra elegantemente y registra el evento
4. **Verificación**: `cat /tmp/my_daemon.log` → confirma que funcionó

---

## Ejemplo de Salida del Log

```
Daemon iniciado con PID 9216
Daemon ejecutándose activamente en segundo plano...
Daemon ejecutándose activamente en segundo plano...
Daemon ejecutándose activamente en segundo plano...
Daemon recibió SIGTERM y se está cerrando
```

---

## Notas Importantes

- El archivo de log se guarda en **`/tmp/my_daemon.log`** (no en `/var/log` porque requiere permisos de root)
- El daemon se desvincula de la terminal, así que la ejecución `./daemon` retorna inmediatamente
- Usar `kill -TERM` es mejor que `kill -KILL` porque permite que el daemon se cierre de forma ordenada
- El proceso no aparecerá en la terminal, pero sigue vivo en segundo plano
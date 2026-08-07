# Ejemplo Nueva Syscall

Syscall personalizada llamada `get_process_info()` dentro del arbol fuente
real del kernel de Linux. No es un kernel completo ni arrancable por si mismo:
solo contiene los archivos que un estudiante deberia crear o modificar dentro
de su entrega.

## Que incluye

- `include/linux/get_process_info.h`: estructura compartida entre kernel y usuario.
- `kernel/get_process_info.c`: implementacion de la syscall.
- `arch/x86/entry/syscalls/syscall_64.tbl`: entrada de ejemplo para x86_64.
- `kernel/Makefile`: fragmento donde se agrega el nuevo objeto.
- `test_get_process_info.c`: programa de usuario que invoca la syscall.

## Para integrar esto en tu propio arbol del kernel

1. Copia `include/linux/get_process_info.h` al arbol fuente del kernel.
2. Copia `kernel/get_process_info.c` dentro de `kernel/`.
3. Agrega la linea de la syscall en `arch/x86/entry/syscalls/syscall_64.tbl`.
4. Agrega el prototipo en `include/linux/syscalls.h`.
5. Incluye `#include <linux/get_process_info.h>` en `include/linux/syscalls.h`.
6. Modifica `kernel/Makefile` para sumar `obj-y += get_process_info.o`.
7. Recompila e instala el kernel completo.
8. Reinicia y verifica que el kernel nuevo este activo.

## Compilacion del programa de prueba

```bash
gcc test_get_process_info.c -o test_get_process_info
```

## Ejecucion

```bash
./test_get_process_info
```

El numero `463` se usa aqui como valor de ejemplo. En un arbol real del kernel,
debes comprobar cual es el numero libre correcto antes de agregar la syscall.

## Notas pedagogicas

La syscall devuelve al espacio de usuario informacion basica del proceso en
ejecucion: nombre del ejecutable, PID, PPID y hora real del sistema. La idea es
mostrar de forma sencilla como se cruzan la frontera kernel-usuario, como se
usa `copy_to_user()` y por que `task_struct` representa el PCB interno del
kernel.

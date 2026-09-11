# Manejo de task_struct y comparación RSS vs VSZ

## Manejo de task_struct

Este modulo de kernel muestra operaciones comunes sobre `task_struct`:

- Inspeccionar el proceso actual (`current`)
- Inspeccionar el padre de un proceso (`real_parent`)
- Buscar un proceso por PID (`find_vpid` + `pid_task`)
- Recorrer la lista global de procesos (`for_each_process`)
- Recorrer el grupo de hilos de un proceso (`for_each_thread`)

### Archivos

- `task.c`: modulo LKM
- `Makefile`: compilacion del modulo

### Compilar

```bash
cd ./Clase9/modulo
make
```

### Cargar modulo

```bash
sudo insmod task.ko
```

Con parametros:

```bash
sudo insmod task.ko target_pid=1
```

### Ver logs del kernel

```bash
dmesg | tail -n 100
```

Filtrar solo el modulo:

```bash
dmesg | grep task_lkm
```

### Descargar modulo

```bash
sudo rmmod task
```

### Limpiar compilacion

```bash
make clean
```

### Parametros disponibles

- `target_pid` (int): PID objetivo para inspeccionar. `-1` para omitir.
- `max_tasks` (int): cantidad maxima de procesos a listar.
- `show_threads` (bool): `1` para listar hilos del PID objetivo.

### Ejemplo de flujo de prueba

1. Obtener un PID de usuario:
```bash
pgrep -n bash
```

2. Cargar modulo con ese PID:
```bash
sudo insmod task.ko target_pid=<PID>
```

3. Revisar salida:
```bash
dmesg | grep task_lkm
```

4. Descargar modulo:
```bash
sudo rmmod task
```

## RSS vs VSZ

- RSS (Resident Set Size): cantidad de memoria física que un proceso está usando actualmente.
- VSZ (Virtual Memory Size): cantidad total de memoria virtual que un proceso ha reservado,

### Archivo

- `rss_vs_vss.c`: programa en C para comparar RSS y VSZ

### Compilar

```bash
gcc -o rss_vs_vss rss_vs_vss.c
```

### Ejecutar

```bash
./rss_vs_vss
```


# NOTA

Para instalar smem el cual es una herramienta para reportar el uso de memoria de los procesos, se puede usar el siguiente comando:

```bash
sudo apt update && sudo apt install smem
```
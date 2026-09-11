# RSS vs VSZ y Gestión de Memoria

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

---

## Gestión de Memoria

Demuestra las funciones fundamentales de asignación y liberación de memoria:
- `malloc()` - Asignación dinámica
- `calloc()` - Asignación e inicialización a cero
- `realloc()` - Redimensionamiento de memoria
- Detección de fugas de memoria

### Compilación y Ejecución

* 1. Compilarlo:

```bash
gcc -Wall -Wextra -O2 -std=c99 gestion_memoria.c -o gestion_memoria
```

* 2. Ejecutar el programa:

```bash
./gestion_memoria
```

* 3. Ejecutar con Valgrind para detectar fugas de memoria:
```bash
valgrind --leak-check=full ./gestion_memoria leak
```

---

### Requisitos

- GCC
- valgrind (para detectar fugas de memoria)

```bash
sudo apt-get install gcc valgrind
sudo apt-get install gcc valgrind leak
```

---

## Fallo de Paginas

### Ejemplo:

Demuestra cómo el sistema operativo maneja los fallos de página, mostrando la asignación de memoria y el proceso de paginación.

### Compilación y Ejecución

* 1. Compilarlo:

```bash
gcc fallo_pagina.c -o fallo_pagina
```

* 2. Ejecutar el programa:

```bash
./fallo_pagina
```

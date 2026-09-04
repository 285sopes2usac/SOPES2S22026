

# Vigía de Buzón — Demo de Hilos de Kernel

Módulo de kernel de ejemplo que ilustra el uso de un **hilo de kernel** (`kthread`) para vigilar de forma periódica y autónoma el contenido de un buzón compartido en memoria, detectando una palabra clave y reportándola en el log del sistema.


## Compilación

```bash
make
```

Esto genera `vigia_buzon.ko` junto con archivos intermedios (`.o`, `.mod.c`, etc.).

## Uso

**1. Cargar el módulo:**
```bash
sudo insmod vigia_buzon.ko
```

**2. Dejar el log del kernel visible en otra terminal (o en segundo plano):**
```bash
sudo dmesg -wH
```

**3. Iniciar el vigía indicando la palabra clave a buscar:**
```bash
echo "sospechoso" > /proc/vigia_control
```

**4. Escribir contenido en el buzón:**
```bash
echo "todo normal" > /proc/buzon
# No genera ninguna detección

echo "proceso sospechoso detectado" > /proc/buzon
# Aparece en dmesg: "vigia: se detectó 'sospechoso' en el buzón -> ..."
```

**5. Detener el vigía de forma segura:**
```bash
echo "stop" > /proc/vigia_control
```

**6. Descargar el módulo:**
```bash
sudo rmmod vigia_buzon
```

## Salida esperada en `dmesg`

```
vigia_buzon: módulo cargado.
vigia: iniciado, buscando 'sospechoso'
vigia: se detectó 'sospechoso' en el buzón -> "proceso sospechoso detectado"
vigia: detenido de forma segura.
vigia_buzon: módulo descargado.
```

## Cómo funciona

- El módulo expone dos entradas en `/proc`:
  - `/proc/vigia_control`: inicia (`echo "palabra" >`) o detiene (`echo "stop" >`) el hilo de vigilancia.
  - `/proc/buzon`: recibe el contenido a inspeccionar.
- Al iniciar, se lanza un hilo de kernel con `kthread_run()` que revisa el buzón cada segundo, buscando la palabra clave con `strnstr()`.
- El acceso al buzón compartido está protegido con un `mutex` (`DEFINE_MUTEX`) para evitar condiciones de carrera si varios procesos escriben al mismo tiempo.
- Al detener el vigía, se usa `kthread_stop()` para terminar el hilo de forma segura, evitando dejarlo huérfano.

## Limpieza

```bash
make clean
```
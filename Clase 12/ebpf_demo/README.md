# Demo eBPF: `openat_watch`

Sensor eBPF mínimo que intercepta `sys_enter_openat`, envía metadatos al espacio de
usuario por un **ring buffer** y muestra los tres patrones que necesita el proyecto:

| Pieza | Dónde está |
|---|---|
| Tracepoint de syscall | `openat_watch.bpf.c`, `SEC("tracepoint/syscalls/sys_enter_openat")` |
| Mapa de configuración escrito desde usuario | mapa `config` + `bpf_map_update_elem()` en `openat_watch.c` |
| Filtro dentro del kernel | bucle acotado que compara el nombre del proceso (`comm`) |
| Ignorar al propio daemon | `daemon_pid` en el mapa de configuración |
| Ring buffer kernel → usuario | mapa `rb` + `ring_buffer__poll()` |
| Activar/desactivar sin recargar | `kill -USR1 <pid>` cambia `enabled` en el mapa |

> **Qué NO hace (queda para el proyecto):** filtrar por ruta de directorio, vigilar
> `write`/creación de archivos, calcular SHA-256, comparar con la blacklist, clasificar
> alertas, cuarentena, y adjuntar/desadjuntar el programa (`bpf_link__destroy`).

## Requisitos

```bash
sudo apt install clang libbpf-dev libelf-dev zlib1g-dev
```

Verifica el kernel (6.12.69 del proyecto):

```bash
zcat /proc/config.gz | grep -E "CONFIG_BPF=|CONFIG_BPF_SYSCALL|CONFIG_BPF_EVENTS|CONFIG_FTRACE_SYSCALLS|CONFIG_DEBUG_INFO_BTF="
# o: grep ... /boot/config-$(uname -r)
ls /sys/kernel/tracing/events/syscalls/sys_enter_openat/format   # debe existir
```

Si el `ls` falla, falta `CONFIG_FTRACE_SYSCALLS` o tracefs sin montar
(`sudo mount -t tracefs nodev /sys/kernel/tracing`).

## Compilar

```bash
make
```

Genera `openat_watch.bpf.o` (programa del kernel, compilado con Clang) y `openat_watch`
(loader en espacio de usuario, enlazado con libbpf). Ejecútalo desde esta carpeta: el
loader abre `openat_watch.bpf.o` por ruta relativa.

## Demo

**A. Funcionamiento normal.** Terminal 1:

```bash
sudo ./openat_watch -c cat
```

Terminal 2:

```bash
cat /etc/hostname
cat /etc/passwd
```

Terminal 1 muestra un evento por cada `openat` de `cat` (incluidas las librerías que
carga el propio `cat`).

**B. Activar/desactivar en caliente.** Con el sensor corriendo:

```bash
sudo kill -USR1 <pid que imprime el programa>
```

Los eventos se detienen y vuelven al repetirlo. Es un cambio en el mapa, no hay que
recargar nada.

**C. El bucle infinito (lo que pasa si no ignoras al daemon).** El daemon, para
analizar un archivo, lo abre; esa apertura es otro `openat`:

```bash
# Sin ignorar al daemon: se retroalimenta hasta el límite de 20 eventos
sudo ./openat_watch -c openat_watch -n -m 20

# Ignorando al daemon (por defecto): silencio, no hay bucle
sudo ./openat_watch -c openat_watch
```
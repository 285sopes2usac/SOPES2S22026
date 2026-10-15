// SPDX-License-Identifier: GPL-2.0
/*
 * Sensor eBPF de ejemplo: intercepta sys_enter_openat y envía metadatos
 * (pid, uid, comm, ruta) al espacio de usuario por un ring buffer.
 *
 * Piezas que se ilustran:
 *   1. Tracepoint de syscall.
 *   2. Mapa de configuración escrito desde el espacio de usuario.
 *   3. Filtrado en el kernel (aquí por nombre de proceso, NO por ruta).
 *   4. Ignorar al propio daemon (evita el bucle infinito).
 *   5. Ring buffer hacia el espacio de usuario.
 */
#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>
#include "openat_watch.h"

/* Contexto del tracepoint syscalls:sys_enter_openat.
 * Ver: /sys/kernel/tracing/events/syscalls/sys_enter_openat/format */
struct sys_enter_ctx {
    __u64 common;           /* campos comunes del tracepoint (8 bytes) */
    long  syscall_nr;
    unsigned long args[6];  /* args[0]=dfd, args[1]=filename, args[2]=flags */
};

/* Mapa de configuración: una sola entrada (clave 0). */
struct {
    __uint(type, BPF_MAP_TYPE_ARRAY);
    __uint(max_entries, 1);
    __type(key, __u32);
    __type(value, struct config);
} config SEC(".maps");

/* Ring buffer: 256 KB (múltiplo del tamaño de página). */
struct {
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 256 * 1024);
} rb SEC(".maps");

SEC("tracepoint/syscalls/sys_enter_openat")
int handle_openat(struct sys_enter_ctx *ctx)
{
    __u32 key = 0;
    struct config *cfg = bpf_map_lookup_elem(&config, &key);

    /* El verificador exige comprobar el NULL antes de usar el puntero. */
    if (!cfg || !cfg->enabled)
        return 0;

    /* Ignorar al propio daemon: si no, cuando el daemon abra el archivo
     * para analizarlo generaría un nuevo evento, y así sin fin. */
    __u32 pid = bpf_get_current_pid_tgid() >> 32;
    if (cfg->daemon_pid && pid == cfg->daemon_pid)
        return 0;

    /* Filtro por nombre de proceso (bucle acotado: el verificador lo exige). */
    if (cfg->comm[0] != '\0') {
        char comm[COMM_LEN];
        bpf_get_current_comm(comm, sizeof(comm));
        #pragma unroll
        for (int i = 0; i < COMM_LEN; i++) {
            if (comm[i] != cfg->comm[i])
                return 0;
            if (comm[i] == '\0')
                break;
        }
    }

    struct event *e = bpf_ringbuf_reserve(&rb, sizeof(*e), 0);
    if (!e)
        return 0;           /* buffer lleno: se pierde el evento */

    e->pid = pid;
    e->uid = (__u32)bpf_get_current_uid_gid();
    bpf_get_current_comm(e->comm, sizeof(e->comm));
    /* La ruta vive en memoria de usuario: se copia con el helper adecuado. */
    bpf_probe_read_user_str(e->filename, sizeof(e->filename),
                            (const char *)ctx->args[1]);

    bpf_ringbuf_submit(e, 0);
    return 0;
}

char LICENSE[] SEC("license") = "GPL";

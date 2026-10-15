// SPDX-License-Identifier: GPL-2.0
/*
 * Loader / "daemon" de ejemplo para openat_watch.bpf.o (usa libbpf).
 *
 *   sudo ./openat_watch [-c comm] [-n] [-m max]
 *     -c comm  nombre de proceso a vigilar (por defecto "cat")
 *     -n       NO ignorar al propio daemon (demuestra el bucle infinito)
 *     -m max   terminar tras 'max' eventos (0 = sin límite)
 *   kill -USR1 <pid>   activa/desactiva el envío de eventos (mapa de config)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include "openat_watch.h"

static volatile sig_atomic_t stop;
static volatile sig_atomic_t toggle;
static unsigned long total, max_events;

static void on_term(int s)   { (void)s; stop = 1; }
static void on_usr1(int s)   { (void)s; toggle = 1; }

/* Simula lo que hará el daemon real: ir al disco y abrir el archivo
 * reportado (por ejemplo, para calcular su SHA-256). Esta apertura genera
 * OTRO evento openat si no se ignora al daemon. */
static void analyze_file(const char *path)
{
    char b[1];
    int fd = open(path, O_RDONLY);
    if (fd < 0)
        return;
    if (read(fd, b, sizeof(b)) < 0)
        perror("read");
    close(fd);
}

static int handle_event(void *ctx, void *data, size_t len)
{
    const struct event *e = data;
    (void)ctx;
    if (len < sizeof(*e))
        return 0;

    printf("[evento %lu] pid=%u uid=%u comm=%s archivo=%s\n",
           ++total, e->pid, e->uid, e->comm, e->filename);
    analyze_file(e->filename);

    if (max_events && total >= max_events) {
        printf("-- límite de %lu eventos alcanzado, saliendo --\n", max_events);
        stop = 1;
    }
    return 0;
}

int main(int argc, char **argv)
{
    struct config cfg = { .enabled = 1, .daemon_pid = getpid() };
    const char *comm = "cat";
    int opt, err = 0;

    while ((opt = getopt(argc, argv, "c:nm:")) != -1) {
        switch (opt) {
        case 'c': comm = optarg; break;
        case 'n': cfg.daemon_pid = 0; break;
        case 'm': max_events = strtoul(optarg, NULL, 10); break;
        default:
            fprintf(stderr, "uso: %s [-c comm] [-n] [-m max]\n", argv[0]);
            return 1;
        }
    }
    strncpy(cfg.comm, comm, COMM_LEN - 1);

    signal(SIGINT, on_term);
    signal(SIGTERM, on_term);
    signal(SIGUSR1, on_usr1);

    struct bpf_object *obj = bpf_object__open_file("openat_watch.bpf.o", NULL);
    if (!obj) {
        fprintf(stderr, "no se pudo abrir el objeto eBPF: %s\n", strerror(errno));
        return 1;
    }
    if ((err = bpf_object__load(obj))) {
        fprintf(stderr, "error al cargar (¿sudo? ¿CONFIG_BPF_SYSCALL?): %s\n",
                strerror(-err));
        goto out_obj;
    }

    /* 1) Escribir la configuración en el mapa ANTES de adjuntar. */
    struct bpf_map *cmap = bpf_object__find_map_by_name(obj, "config");
    struct bpf_map *rmap = bpf_object__find_map_by_name(obj, "rb");
    if (!cmap || !rmap) {
        fprintf(stderr, "no se encontraron los mapas\n");
        goto out_obj;
    }
    int cfd = bpf_map__fd(cmap);
    __u32 key = 0;
    if (bpf_map_update_elem(cfd, &key, &cfg, BPF_ANY)) {
        perror("bpf_map_update_elem");
        goto out_obj;
    }

    /* 2) Adjuntar el programa al tracepoint. */
    struct bpf_program *prog = bpf_object__find_program_by_name(obj, "handle_openat");
    struct bpf_link *link = prog ? bpf_program__attach(prog) : NULL;
    if (!link) {
        fprintf(stderr, "no se pudo adjuntar al tracepoint: %s\n"
                "Revisa: ls /sys/kernel/tracing/events/syscalls/sys_enter_openat\n"
                "(si no existe: CONFIG_FTRACE_SYSCALLS o tracefs sin montar)\n",
                strerror(errno));
        goto out_obj;
    }

    /* 3) Consumir el ring buffer. */
    struct ring_buffer *rb = ring_buffer__new(bpf_map__fd(rmap), handle_event, NULL, NULL);
    if (!rb) {
        fprintf(stderr, "no se pudo crear el ring buffer\n");
        goto out_link;
    }

    printf("Vigilando openat de procesos \"%s\" | ignora al daemon (pid %u): %s | pid=%d\n",
           cfg.comm, cfg.daemon_pid, cfg.daemon_pid ? "SI" : "NO", getpid());
    printf("Ctrl+C para salir, kill -USR1 %d para activar/desactivar.\n", getpid());

    /* Autoprueba: una apertura del propio daemon. Si el daemon no se ignora
     * y se vigila su mismo nombre, esto dispara el bucle. */
    analyze_file("/etc/hostname");

    while (!stop) {
        if (toggle) {
            toggle = 0;
            cfg.enabled = !cfg.enabled;
            bpf_map_update_elem(cfd, &key, &cfg, BPF_ANY);
            printf("-- envío de eventos %s --\n", cfg.enabled ? "ACTIVADO" : "DESACTIVADO");
        }
        err = ring_buffer__poll(rb, 100 /* ms */);
        if (err < 0 && err != -EINTR) {
            fprintf(stderr, "error en poll: %d\n", err);
            break;
        }
    }

    ring_buffer__free(rb);
out_link:
    bpf_link__destroy(link);
out_obj:
    bpf_object__close(obj);
    return err < 0 ? 1 : 0;
}

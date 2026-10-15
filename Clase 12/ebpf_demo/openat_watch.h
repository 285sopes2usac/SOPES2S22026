/* Estructuras compartidas entre el programa eBPF (kernel) y el loader (usuario). */
#ifndef OPENAT_WATCH_H
#define OPENAT_WATCH_H

#include <linux/types.h>

#define COMM_LEN  16
#define FNAME_LEN 256

/* Configuración que escribe el loader y lee el programa eBPF (mapa "config"). */
struct config {
    __u32 enabled;          /* 1 = enviar eventos, 0 = descartarlos         */
    __u32 daemon_pid;       /* PID a ignorar (0 = no ignorar a nadie)        */
    char  comm[COMM_LEN];   /* solo procesos con este nombre ("" = todos)    */
};

/* Metadatos que viajan por el ring buffer. NO se calcula nada en el kernel. */
struct event {
    __u32 pid;
    __u32 uid;
    char  comm[COMM_LEN];
    char  filename[FNAME_LEN];
};

#endif

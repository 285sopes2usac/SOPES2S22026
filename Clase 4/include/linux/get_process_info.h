#ifndef _LINUX_GET_PROCESS_INFO_H
#define _LINUX_GET_PROCESS_INFO_H

#include <linux/types.h>

/*
 * Usamos tipos de ancho fijo para que la ABI de la estructura sea estable
 * entre kernel y espacio de usuario, sin depender del tamano de int, long o
 * de la arquitectura concreta.
 */
#define GPI_COMM_LEN 16

/*
 * Estructura compartida entre kernel y usuario para transportar la
 * informacion basica del proceso actual.
 */
struct process_info {
	char   comm[GPI_COMM_LEN];
	__u32  pid;
	__u32  ppid;
	__s64  current_time;
};

#endif /* _LINUX_GET_PROCESS_INFO_H */

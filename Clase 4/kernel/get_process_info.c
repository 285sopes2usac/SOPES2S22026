// SPDX-License-Identifier: GPL-2.0
/*
 * kernel/get_process_info.c
 *
 * Syscall educativa get_process_info().
 * Expone informacion basica del proceso en ejecucion para fines de estudio.
 */

#include <linux/errno.h>
#include <linux/get_process_info.h>
#include <linux/kernel.h>
#include <linux/pid.h>
#include <linux/rcupdate.h>
#include <linux/sched.h>
#include <linux/sched/task.h>
#include <linux/string.h>
#include <linux/syscalls.h>
#include <linux/timekeeping.h>
#include <linux/uaccess.h>

/*
 * SYSCALL_DEFINE1 crea la entrada real de la syscall en el kernel y la deja
 * integrada en el mecanismo normal de despacho del sistema.
 */
SYSCALL_DEFINE1(get_process_info, struct process_info __user *, info)
{
	struct process_info kernel_info;

	/*
	 * Primero validamos el puntero de usuario. Nunca se debe confiar en una
	 * direccion que venga desde espacio de usuario.
	 */
	if (info == NULL)
		return -EINVAL;

	if (!access_ok(info, sizeof(kernel_info)))
		return -EFAULT;

	/*
	 * Limpiamos la estructura local para evitar filtrar basura de pila o bytes
	 * no inicializados al espacio de usuario.
	 */
	memset(&kernel_info, 0, sizeof(kernel_info));

	/*
	 * current apunta al task_struct del proceso que esta ejecutando la syscall.
	 * get_task_comm() copia el nombre del ejecutable de forma segura.
	 */
	get_task_comm(kernel_info.comm, current);

	/*
	 * task_pid_vnr() devuelve el PID visible desde el namespace del llamante.
	 * Es preferible a leer current->pid directamente en sistemas con namespaces.
	 */
	kernel_info.pid = task_pid_vnr(current);

	/*
	 * El padre se lee bajo RCU para respetar la concurrencia del arbol de
	 * procesos. Asi evitamos una referencia inestable a real_parent.
	 */
	rcu_read_lock();
	kernel_info.ppid = task_pid_vnr(current->real_parent);
	rcu_read_unlock();

	/*
	 * Tomamos la hora real del sistema en segundos desde epoch.
	 * ktime_get_real_seconds() es la API moderna y evita problemas de Y2038.
	 */
	kernel_info.current_time = ktime_get_real_seconds();

	/*
	 * copy_to_user() es la unica forma segura de escribir en memoria de
	 * usuario desde el kernel.
	 */
	if (copy_to_user(info, &kernel_info, sizeof(kernel_info)))
		return -EFAULT;

	return 0;
}

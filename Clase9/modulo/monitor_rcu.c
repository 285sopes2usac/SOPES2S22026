#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched/signal.h>
#include <linux/rcupdate.h> // Para rcu_read_lock()

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SOPES2");
MODULE_DESCRIPTION("LKM para leer task_struct e hijos de forma segura con RCU");
MODULE_VERSION("1.0");

// Permite pasar un PID como parámetro al cargar el módulo (ej. insmod monitor_rcu.ko target_pid=1234)
static int target_pid = 1; 
module_param(target_pid, int, 0644);

static int __init monitor_rcu_init(void) {
    struct task_struct *task;
    struct list_head *list;
    struct task_struct *child;

    printk(KERN_INFO "SO2: Iniciando lectura avanzada de procesos para PID %d\n", target_pid);

    // INICIO DE ZONA CRÍTICA DE LECTURA (RCU)
    // Garantiza que la estructura no sea eliminada por otro hilo mientras la leemos
    rcu_read_lock();
    
    // Buscar la tarea por su PID
    task = pid_task(find_vpid(target_pid), PIDTYPE_PID);
    if (!task) {
        printk(KERN_ERR "SO2: Error -> Proceso con PID %d no encontrado.\n", target_pid);
        rcu_read_unlock();
        return -ENODEV;
    }

    printk(KERN_INFO "SO2: Proceso Padre -> PID: %d, Nombre (comm): %s\n", task->pid, task->comm);
    printk(KERN_INFO "SO2: --- Iterando sobre lista Children (Hijos) ---\n");

    // Iterar sobre la lista enlazada de hijos usando el enlace 'sibling'
    list_for_each(list, &task->children) {
        child = list_entry(list, struct task_struct, sibling);
        printk(KERN_INFO "SO2: \tHijo -> PID: %d, Nombre: %s\n", child->pid, child->comm);
    }

    // FIN DE ZONA CRÍTICA DE LECTURA (RCU)
    rcu_read_unlock();

    return 0;
}

static void __exit monitor_rcu_exit(void) {
    printk(KERN_INFO "SO2: Modulo de monitoreo RCU descargado.\n");
}

module_init(monitor_rcu_init);
module_exit(monitor_rcu_exit);
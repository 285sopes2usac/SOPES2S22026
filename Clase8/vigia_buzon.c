// vigia_buzon.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>

#define TAM_BUZON 128

static struct task_struct *hilo_vigia;
static char buzon[TAM_BUZON];
static char palabra_buscada[64];
static DEFINE_MUTEX(mutex_buzon);
static int vigia_activo = 0;

// Hilo que revisa el buzón periódicamente
static int funcion_vigia(void *data)
{
    while (!kthread_should_stop()) {
        mutex_lock(&mutex_buzon);
        if (strlen(buzon) > 0 && strnstr(buzon, palabra_buscada, TAM_BUZON)) {
            printk(KERN_INFO "vigia: se detectó '%s' en el buzón -> \"%s\"\n",
                   palabra_buscada, buzon);
            memset(buzon, 0, TAM_BUZON); // se "consume" el mensaje
        }
        mutex_unlock(&mutex_buzon);
        msleep(1000); // revisa cada 1 segundo
    }
    return 0;
}

// echo "texto" > /proc/buzon  --> escribe en el buzón
static ssize_t escribir_buzon(struct file *f, const char __user *buf, size_t len, loff_t *off)
{
    size_t bytes = min(len, (size_t)(TAM_BUZON - 1));

    mutex_lock(&mutex_buzon);
    memset(buzon, 0, TAM_BUZON);
    if (copy_from_user(buzon, buf, bytes)) {
        mutex_unlock(&mutex_buzon);
        return -EFAULT;
    }
    mutex_unlock(&mutex_buzon);
    return len;
}

static const struct proc_ops fops_buzon = {
    .proc_write = escribir_buzon,
};

// echo "palabra" > /proc/vigia_control  --> inicia el vigía
// echo "stop" > /proc/vigia_control     --> lo detiene
static ssize_t controlar_vigia(struct file *f, const char __user *buf, size_t len, loff_t *off)
{
    char entrada[64];
    size_t bytes = min(len, sizeof(entrada) - 1);

    memset(entrada, 0, sizeof(entrada));
    if (copy_from_user(entrada, buf, bytes))
        return -EFAULT;

    // quitar salto de línea si viene de echo
    if (entrada[bytes - 1] == '\n')
        entrada[bytes - 1] = '\0';

    if (strcmp(entrada, "stop") == 0) {
        if (vigia_activo) {
            kthread_stop(hilo_vigia);
            vigia_activo = 0;
            printk(KERN_INFO "vigia: detenido de forma segura.\n");
        }
        return len;
    }

    if (vigia_activo) {
        printk(KERN_INFO "vigia: ya hay un vigía activo, deténlo primero.\n");
        return len;
    }

    strscpy(palabra_buscada, entrada, sizeof(palabra_buscada));
    hilo_vigia = kthread_run(funcion_vigia, NULL, "hilo_vigia_demo");
    if (IS_ERR(hilo_vigia)) {
        printk(KERN_ERR "vigia: no se pudo crear el hilo.\n");
        return -ENOMEM;
    }
    vigia_activo = 1;
    printk(KERN_INFO "vigia: iniciado, buscando '%s'\n", palabra_buscada);
    return len;
}

static const struct proc_ops fops_control = {
    .proc_write = controlar_vigia,
};

static int __init vigia_init(void)
{
    proc_create("buzon", 0222, NULL, &fops_buzon);
    proc_create("vigia_control", 0222, NULL, &fops_control);
    printk(KERN_INFO "vigia_buzon: módulo cargado.\n");
    return 0;
}

static void __exit vigia_exit(void)
{
    if (vigia_activo)
        kthread_stop(hilo_vigia);
    remove_proc_entry("buzon", NULL);
    remove_proc_entry("vigia_control", NULL);
    printk(KERN_INFO "vigia_buzon: módulo descargado.\n");
}

module_init(vigia_init);
module_exit(vigia_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Curso SO2");
MODULE_DESCRIPTION("Demo: hilo de kernel vigilando un buzón compartido");
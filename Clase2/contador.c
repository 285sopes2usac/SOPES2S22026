#include <linux/init.h>     // Para las macros __init y __exit
#include <linux/kernel.h>   // Para printk y KERN_INFO
#include <linux/module.h>   // Para la funcionalidad de módulos

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SOPES2");
MODULE_DESCRIPTION("Módulo contador con parámetros");
MODULE_VERSION("1.0");


static int counter = 1;

module_param(counter, int, S_IRUGO);    // Permite leer el valor de counter desde el espacio de usuario

// S_IRUGO = 0444, solo lectura para el usuario, no se puede modificar desde el espacio de usuario

static int __init m_init(void)
{
    printk("[CONTADOR] Módulo de pruebas cargado\n");
    for (int i = 0; i < counter; ++i) {
        pr_info("[CONTADOR] Contador: %d \n", i + 1);
    }
    return 0;
}

static void __exit m_exit(void)
{
    printk(KERN_DEBUG "[CONTADOR] Módulo de pruebas descargado\n");
}

module_init(m_init);
module_exit(m_exit);
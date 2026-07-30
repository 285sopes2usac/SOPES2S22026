#include <linux/module.h> // Necesario para módulos de kernel
#include <linux/init.h> // Necesario para funciones de inicialización y limpieza
#include <linux/input.h> // Necesario para manejar dispositivos de entrada
#include <linux/proc_fs.h> // Necesario para manejar el sistema de archivos /proc
#include <linux/uaccess.h> // Necesario para funciones de acceso a memoria de usuario

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SOPES2");
MODULE_DESCRIPTION("Módulo para mover el teclado virtual con /proc");
MODULE_VERSION("1.0");

static struct input_dev *vkeyboard; // Puntero a la estructura keyboard
static struct proc_dir_entry *proc_entry; // Puntero al archivo que crearemos en /proc

#define PROC_NAME "teclado_virtual" // Nombre del archivo dentro de /proc
#define BUF_LEN  64 // Tamaño maximo del mensaje que aceptaremos

/**
Funcion que se activa cuando se escribe en el archivo /proc/vkeyboard_control
@param file puntero al archivo proc
@param buffer puntero al buffer de datos que se escriben, pertenece al espacio de usuario
@param count cantidad de bytes que se escriben
@param pos posición actual en el archivo

**funcion estandar
*/
static ssize_t keyboard_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
    char buf[BUF_LEN]; // Buffer temporal para almacenar los datos recibidos del usuario
    int keycode; // Variable para almacenar el keycode que se recibirá del usuario

    // Verificamos si el tamaño del buffer es válido,  se espera uno de 64 o menos, si fuera mas grande, no lo aceptamos
    if (count > BUF_LEN - 1)
        return -EINVAL;

    // Copiamos el buffer de usuario al buffer del kernel de forma segura
    if (copy_from_user(buf, buffer, count))
        return -EFAULT;

    buf[count] = '\0'; // Agregamos un nulo al final del buffer, este valor especial indica el final de una cadena en C


    /** sscanf: recibe un buffer de datos y los interpreta segun el valor que se indica
    en este caso, mi cadena trae un entero el valor key lo quiero guardar en mi variable key, 
    por eso paso su referencia */
    int parsed = sscanf(buf, "%d", &keycode);
    if (parsed < 1)
        return -EINVAL;

    // Movemos nuestro keyboard virtual de manera relativa en X y Y con los valores recibidos
    input_report_key(vkeyboard, keycode, 1); // Reportamos la presion de la tecla al sistema
    input_sync(vkeyboard);                   // Despacha los eventos del keyboard al sistema
    input_report_key(vkeyboard, keycode, 0); // Reportamos la liberacion de la tecla al sistema
    input_sync(vkeyboard);                   // Despacha los eventos del keyboard al sistema

    pr_info("[TECLADO VIRTUAL] Presion de tecla: %d\n", keycode);
    return count;
}


/**
Aquí "conectamos" nuestra función keyboard_proc_write al sistema. Le decimos al kernel: "Cuando alguien intente escribir en este archivo, usa mi función".
**/
static const struct proc_ops proc_file_ops = {
    .proc_write = keyboard_proc_write, // Asociamos nuestra función de escritura
};


/** Funcion de Carga del keyboard virtual */
static int __init keyboard_init(void)
{
    int err, code;

    vkeyboard = input_allocate_device(); // Solicitamos espacio en memoria para nuestro dispositivo virtual
    if (!vkeyboard) {
        pr_err("[TECLADO VIRTUAL] No se pudo alocar el dispositivo input\n");
        return -ENOMEM;
    }

    vkeyboard->name = "teclado_virtual";        // Le asignamos un nombre
    vkeyboard->phys = "vmd/input1";    // Le damos una ruta "fisica" virtual
    vkeyboard->id.bustype = BUS_VIRTUAL;   // Simularemos un disposito virtual
    vkeyboard->id.vendor  = 0x0007;    // ID de vendedor ficticio
    vkeyboard->id.product = 0x0009;    // ID de producto ficticio
    vkeyboard->id.version = 0x001;     // Versión ficticia del dispositivo

    __set_bit(EV_KEY, vkeyboard->evbit);   // Indicamos que el dispositivo soporta eventos de teclas
    __set_bit(EV_SYN, vkeyboard->evbit);   // Indicamos que el dispositivo soporta eventos de sincronización

    for (code = 0; code <= KEY_MAX && code < KEY_CNT; code++)
        __set_bit(code, vkeyboard->keybit);

    err = input_register_device(vkeyboard); // Registramos el dispositivo input
    
    if (err) {
        pr_err("[TECLADO VIRTUAL] No se pudo registrar el dispositivo input\n");
        input_free_device(vkeyboard);
        return err;
    }

    proc_entry = proc_create(
        PROC_NAME,      // Creamos una entrada en /proc
        0222,           // Permisos de escritura
        NULL,           // Directorio padre, NULL significa raíz
        &proc_file_ops  // Estructura de operaciones del archivo, para asociar nuestra funcion al archivo proc
    );

    if (!proc_entry) {
        input_unregister_device(vkeyboard);
        pr_err("[TECLADO VIRTUAL] No se pudo crear entrada /proc\n");
        return -ENOMEM;
    }

    pr_info("[TECLADO VIRTUAL] Módulo cargado: dispositivo vkeyboard listo\n");
    return 0;
}


// Funcion de Descarga del keyboard
static void __exit keyboard_exit(void)
{
    proc_remove(proc_entry);
    input_unregister_device(vkeyboard);
    pr_info("[TECLADO VIRTUAL] Módulo descargado: Teclado virtual removido\n");
}

module_init(keyboard_init);
module_exit(keyboard_exit);

// Ver el proc: ls -l /proc/keyboard_control
// sudo apt-get install evtest
// sudo evtest
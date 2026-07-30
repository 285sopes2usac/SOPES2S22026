# Clase 3 | Creación modulo teclado virtual

## Instrucciones para ejecutar el módulo de kernel

0. Instala las herramientas de compilacion
   ```bash
   sudo apt -y install build-essential libncurses-dev bison flex libssl-dev libelf-dev
   ```

1. Abre una terminal y navega al directorio del módulo:
    ```bash
    cd ~/modulo/Clase3
    ```

3. Compila el módulo usando `make`:
    ```bash
    make
    ```

4. Inserta el módulo en el kernel:
    ```bash
    sudo insmod teclado_virtual.ko
    ```

5. Verifica que el módulo esté cargado:
    ```bash
    lsmod | grep teclado
    ```

6. Consulta los mensajes del kernel para ver la salida del módulo:
    ```bash
    dmesg | tail
    ```

    Para presionar una tecla, escribe en /proc/teclado_virtual
    ```bash
    echo "10" > /proc/teclado_virtual
    ```
    
    Presiona la tecla "9" que tiene el keycode 10 

7. Para remover el módulo:
    ```bash
    sudo rmmod teclado_virtual.ko
    ```

8. Limpia los archivos generados:
    ```bash
    make clean
    ```

## Instrucciones para ejecutar el programa de prueba

1. Abre una terminal y navega al directorio del programa de prueba:
    ```bash
    cd ~/Clase3
    ```

2. Compila el programa de prueba:
    ```bash
    gcc test.c -o test
    ```

3. Ejecuta el programa de prueba:
    ```bash
    ./test
    ```

* Nota: Asegúrate de que el módulo de kernel esté cargado antes de ejecutar el programa de prueba, ya que este programa interactúa con el módulo para simular la presión de teclas.

## ¿Qué hace este módulo?

Este módulo de kernel es un ejemplo básico que, al ser cargado, permite controlar un teclado virtual, escribiendo en **/proc/teclado_virtual** el keycode de la tecla que se desea presionar. El módulo reporta al sistema la presion y liberacion de la tecla, y muestra un mensaje en el log del kernel cada vez que se presiona una tecla.

## Enlaces de interés

* https://linux.die.net/lkmpg/x769.html
* https://www.kernel.org/doc/html/v4.18/input/event-codes.html#event-codes
* https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h
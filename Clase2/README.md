# Clase 2 | Creación modulo

## Instrucciones para ejecutar el módulo de kernel

0. Instala las herramientas de compilacion

    ```bash
   sudo apt -y install build-essential libncurses-dev bison flex libssl-dev libelf-dev
    ```

1. Abre una terminal y navega al directorio del módulo:

    ```bash
    cd ~/Clase2
    ```

3. Compila el módulo usando `make`:

    ```bash
    make
    ```

4. Inserta el módulo en el kernel:

    ```bash
    sudo insmod contador.ko
    ```

5. Verifica que el módulo esté cargado:

    ```bash
    lsmod | grep contador
    ```

7. Para remover el módulo:

    ```bash
    sudo rmmod contador.ko
    ```

8. Limpia los archivos generados:

    ```bash
    make clean
    ```

## Pasar parámetros al módulo

Puedes pasar parámetros al módulo al momento de cargarlo usando `insmod`:

```bash
sudo insmod contador.ko counter=5
```

Salida esperada en el registro del kernel:

```
[CONTADOR] Módulo de pruebas cargado
[CONTADOR] Contador: 1
[CONTADOR] Contador: 2
[CONTADOR] Contador: 3
[CONTADOR] Contador: 4
[CONTADOR] Contador: 5
```

## ¿Qué hace este módulo?

Este módulo es un ejemplo básico de un módulo de kernel que imprime mensajes en el registro del kernel cuando se carga y se descarga, permitiendo pasar parametros al módulo a través de la línea de comandos.

## Enlaces de interés

* https://linux.die.net/lkmpg/x769.html
* https://linux-kernel-labs.github.io/refs/heads/master/labs/kernel_modules.html
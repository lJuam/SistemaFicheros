/************************************************************************
 * PONTIFICIA UNIVERSIDAD JAVERIANA
 *
 * Materia: Sistemas Operativos
 * Docente: J. Corredor, PhD
 * Autor: Juan David Garzon Ballen
 * Programa: ficherosLab01.c
 * Fecha: 2025-11-11
 * Tema: Listado de archivos y tamaño en directorio
 * -----------------------------------------------
 * Descripcion:
 * Programa en C que lee el nombre de un directorio desde teclado
 * y muestra en pantalla el nombre y tamaño (en bytes) de los ficheros
 * regulares que contiene dicho directorio.
 ***********************************************************************/

/*
 * LIBRERÍA: <sys/types.h>
 * Proporciona definiciones de tipos de datos primitivos utilizados por
 * las llamadas al sistema. Incluye tipos como:
 * - mode_t: para representar modos de archivo
 * - off_t: para tamaños y offsets de archivo
 * - pid_t: para IDs de proceso
 * Es fundamental para garantizar portabilidad entre diferentes sistemas Unix/Linux
 */
#include <sys/types.h>

/*
 * LIBRERÍA: <sys/stat.h>
 * Proporciona funcionalidad para obtener información sobre archivos.
 * Incluye:
 * - struct stat: estructura que contiene metadatos de archivos (tamaño, permisos, fechas, etc.)
 * - stat(): función para obtener información de un archivo
 * - Macros de verificación de tipo: S_ISREG(), S_ISDIR(), S_ISLNK(), etc.
 * - Constantes de permisos: S_IRUSR, S_IWUSR, S_IXUSR, etc.
 */
#include <sys/stat.h>

/*
 * LIBRERÍA: <dirent.h>
 * Proporciona funciones para manipular directorios.
 * Elementos clave:
 * - DIR: tipo opaco que representa un directorio abierto
 * - struct dirent: estructura con información de cada entrada del directorio
 *   - d_name: nombre del archivo/directorio
 *   - d_type: tipo de entrada (en algunos sistemas)
 * - opendir(): abre un directorio para lectura
 * - readdir(): lee la siguiente entrada del directorio
 * - closedir(): cierra un directorio abierto
 */
#include <dirent.h>

/*
 * LIBRERÍA: <stdio.h>
 * Librería estándar de entrada/salida.
 * Funciones utilizadas:
 * - printf(): imprime datos formateados en la salida estándar
 * - fgets(): lee una línea de texto de forma segura (previene desbordamiento)
 */
#include <stdio.h>

/*
 * LIBRERÍA: <stdlib.h>
 * Librería estándar de utilidades generales.
 * Aunque está incluida, no se usa explícitamente en este programa.
 * Comúnmente proporciona: malloc(), free(), exit(), atoi(), etc.
 * Se incluye por buenas prácticas o para posibles extensiones futuras.
 */
#include <stdlib.h>

/*
 * LIBRERÍA: <string.h>
 * Proporciona funciones para manipulación de cadenas de caracteres.
 * Funciones utilizadas:
 * - strlen(): calcula la longitud de una cadena
 * - strcpy(): copia una cadena en otra
 * - strcat(): concatena (añade) una cadena al final de otra
 * - strcmp(): compara dos cadenas lexicográficamente
 */
#include <string.h>

/*
 * LIBRERÍA: <unistd.h>
 * API POSIX para acceso a llamadas al sistema Unix/Linux.
 * Aunque está incluida, no se usa explícitamente en este código.
 * Comúnmente proporciona: read(), write(), close(), chdir(), etc.
 * Se incluye por compatibilidad o para extensiones futuras.
 */
#include <unistd.h>

/*
 * FUNCIÓN PRINCIPAL
 * Punto de entrada del programa
 */
int main() {
    /*
     * DECLARACIÓN DE VARIABLES
     */

    // Puntero a la estructura DIR que representa el directorio abierto
    // DIR es un tipo opaco (no accedemos directamente a sus campos)
    DIR *d;

    // Buffer para almacenar el nombre del directorio ingresado por el usuario
    // Tamaño: 256 bytes (suficiente para la mayoría de nombres de directorio)
    char nomdir[256];

    // Buffer para almacenar la ruta completa de cada archivo
    // Tamaño: 512 bytes (directorio + "/" + nombre_archivo)
    // Tamaño mayor para prevenir desbordamiento con rutas largas
    char nomfich[512];

    // Estructura que almacenará la información (metadatos) de cada archivo
    // Contiene: tamaño, permisos, propietario, fechas, tipo, etc.
    struct stat datos;

    // Puntero a la estructura dirent que representa cada entrada del directorio
    // struct dirent contiene principalmente el campo d_name (nombre de la entrada)
    struct dirent *direc;

    /*
     * SOLICITUD Y LECTURA DEL NOMBRE DEL DIRECTORIO
     */

    // Solicitar al usuario el nombre del directorio
    printf("Introduzca el nombre de un directorio: ");

    // Leer el nombre del directorio de forma segura
    // fgets() es más seguro que gets() porque limita el número de caracteres leídos
    // Parámetros: buffer destino, tamaño máximo, flujo de entrada (stdin = teclado)
    fgets(nomdir, sizeof(nomdir), stdin);

    // Eliminar el carácter de nueva línea '\n' que fgets() captura al final
    // strlen(nomdir)-1 apunta al último carácter (que es '\n')
    // Lo reemplazamos con '\0' (terminador de cadena)
    nomdir[strlen(nomdir)-1] = '\0';

    /*
     * APERTURA DEL DIRECTORIO
     */

    // Intentar abrir el directorio especificado
    // opendir() retorna un puntero DIR* si tiene éxito, o NULL si falla
    // Razones de fallo: directorio no existe, sin permisos de lectura, etc.
    if ((d = opendir(nomdir)) == NULL) {
        // Si opendir() falla, informar al usuario y terminar el programa
        printf("El directorio no existe\n");
        return -1;  // Retornar -1 indica error
    }

    /*
     * ITERACIÓN SOBRE LAS ENTRADAS DEL DIRECTORIO
     */

    // Bucle que lee cada entrada del directorio hasta que no haya más
    // readdir() retorna un puntero a struct dirent con la siguiente entrada,
    // o NULL cuando se han leído todas las entradas
    while ((direc = readdir(d)) != NULL) {

        /*
         * FILTRADO DE ENTRADAS ESPECIALES
         */

        // Ignorar las entradas especiales "." y ".."
        // ".": representa el directorio actual
        // "..": representa el directorio padre
        // strcmp() retorna 0 cuando las cadenas son iguales
        if(strcmp(direc->d_name, ".") == 0 || strcmp(direc->d_name, "..") == 0)
            continue;  // Saltar a la siguiente iteración del bucle

        /*
         * CONSTRUCCIÓN DE LA RUTA COMPLETA DEL ARCHIVO
         */

        // La función stat() requiere la ruta completa del archivo,
        // no solo su nombre. Por eso construimos: directorio + "/" + nombre_archivo

        // Paso 1: Copiar el nombre del directorio al buffer nomfich
        strcpy(nomfich, nomdir);

        // Paso 2: Añadir el separador de directorios "/"
        strcat(nomfich, "/");

        // Paso 3: Añadir el nombre del archivo
        // Resultado final: "/ruta/al/directorio/nombre_archivo"
        strcat(nomfich, direc->d_name);

        /*
         * OBTENCIÓN DE INFORMACIÓN DEL ARCHIVO Y FILTRADO POR TIPO
         */

        // stat(): obtiene información detallada del archivo y la almacena en 'datos'
        // Retorna 0 si tiene éxito, -1 si falla (archivo no existe, sin permisos, etc.)
        //
        // S_ISREG(): macro que verifica si el archivo es un archivo regular
        // Tipos de archivo en Unix/Linux:
        // - Regular: archivo normal con datos
        // - Directorio: carpeta que contiene otras entradas
        // - Enlace simbólico: acceso directo a otro archivo
        // - Dispositivo de bloque: disco duro, USB, etc.
        // - Dispositivo de caracteres: terminal, puerto serie, etc.
        // - FIFO (pipe): comunicación entre procesos
        // - Socket: comunicación en red
        //
        // datos.st_mode: campo que contiene el tipo y permisos del archivo
        if(stat(nomfich, &datos) == 0 && S_ISREG(datos.st_mode)) {

            /*
             * MOSTRAR INFORMACIÓN DEL ARCHIVO
             */

            // Imprimir el nombre y tamaño del archivo en formato tabular
            // %s: especificador de formato para cadenas de texto (string)
            // %ld: especificador de formato para enteros largos (long)
            // \t: tabulación para alineación visual
            // datos.st_size: tamaño del archivo en bytes (tipo off_t, tratado como long)
            printf("Nombre: %s\t|\tTamaño: %ld bytes\n", direc->d_name, datos.st_size);
        }
        // Si stat() falla o no es archivo regular, simplemente no se muestra
        // (no hay else, se omite silenciosamente)
    }

    /*
     * CIERRE DEL DIRECTORIO Y FINALIZACIÓN
     */

    // Cerrar el directorio para liberar recursos del sistema
    // Es fundamental cerrar siempre los recursos abiertos para evitar:
    // - Fugas de memoria
    // - Agotamiento de descriptores de archivo
    // - Bloqueos de recursos
    closedir(d);

    // Retornar 0 indica que el programa terminó exitosamente
    return 0;
}

/*****************************************************
 * Observaciones y Conclusiones:
 *
 * Este programa permite listar ficheros regulares de un
 * directorio especificado, mostrando su nombre y tamaño
 * en bytes. Fue desarrollado para entender el uso de
 * llamadas a sistema POSIX para manipulación de
 * directorios y para practicar el manejo de estructuras
 * como dirent.
 *
 * Restricciones: El programa asume un directorio válido
 * y no maneja en profundidad errores complejos o enlaces
 * simbólicos.
 *
 * Posibles mejoras:
 * - Modularizar el código en funciones auxiliares.
 * - Añadir manejo de enlaces simbólicos.
 * - Mejorar robustez y validación de entrada.
 *****************************************************/

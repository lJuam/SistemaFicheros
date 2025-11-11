/***************************************************************************
 * PONTIFICIA UNIVERSIDAD JAVERIANA
 *
 * Materia: Sistemas Operativos
 * Docente: J. Corredor, PhD
 * Autor: Juan David Garzon Ballen
 * Programa: ficherosLab02.c
 * Fecha: 2025-11-11
 * Tema: Información detallada de ficheros y directorios
 * -----------------------------------------------
 * Descripcion:
 * Programa que recibe un nombre de directorio y muestra los nombres
 * de ficheros y directorios que contiene, su modo,
 * si tienen permiso de lectura para el propietario,
 * si son directorios, y para ficheros modificados en los últimos 10 días,
 * muestra su fecha de modificación.
 ***************************************************************************/

/*
 * LIBRERÍA: <stdio.h>
 * Librería estándar de entrada/salida en C.
 * Proporciona funciones para:
 * - printf(): imprimir datos formateados en pantalla
 * - fgets(): leer cadenas de texto de forma segura desde teclado
 * - fprintf(), scanf(), etc.
 */
#include <stdio.h>

/*
 * LIBRERÍA: <string.h>
 * Librería para manipulación de cadenas de caracteres (strings).
 * Funciones utilizadas en este programa:
 * - strlen(): calcula la longitud de una cadena
 * - strcpy(): copia una cadena en otra
 * - strcat(): concatena (añade) una cadena al final de otra
 * - strcmp(): compara dos cadenas y retorna 0 si son iguales
 */
#include <string.h>

/*
 * LIBRERÍA: <time.h>
 * Proporciona funciones y tipos para trabajar con fechas y tiempo.
 * Elementos utilizados:
 * - time_t: tipo de dato para representar tiempo (segundos desde epoch: 1 enero 1970)
 * - time(): obtiene el tiempo actual del sistema
 * - ctime(): convierte un time_t a cadena legible (ej: "Mon Nov 11 14:30:45 2025")
 * Esta librería es crucial para comparar fechas de modificación de archivos
 */
#include <time.h>

/*
 * LIBRERÍA: <dirent.h>
 * API para manipulación de directorios en sistemas Unix/Linux.
 * Proporciona:
 * - DIR: tipo opaco que representa un directorio abierto
 * - struct dirent: estructura con información de cada entrada del directorio
 *   - d_name: nombre del archivo/subdirectorio
 * - opendir(): abre un directorio para lectura
 * - readdir(): lee la siguiente entrada del directorio
 * - closedir(): cierra el directorio y libera recursos
 */
#include <dirent.h>

/*
 * LIBRERÍA: <unistd.h>
 * API POSIX (Portable Operating System Interface) para Unix/Linux.
 * Proporciona acceso a llamadas al sistema operativo.
 * Aunque incluida, no se usa explícitamente aquí.
 * Comúnmente proporciona: read(), write(), close(), access(), chdir(), etc.
 * Se incluye por compatibilidad POSIX o para extensiones futuras.
 */
#include <unistd.h>

/*
 * LIBRERÍA: <sys/stat.h>
 * Proporciona funcionalidad avanzada para obtener información de archivos.
 * Elementos clave utilizados:
 * - struct stat: estructura con metadatos completos de archivos/directorios
 *   - st_mode: tipo de archivo y permisos (en formato octal)
 *   - st_mtime: fecha de última modificación (tipo time_t)
 *   - st_size: tamaño en bytes
 *   - st_uid, st_gid: propietario y grupo
 * - stat(): función que obtiene información de un archivo
 * - Macros de verificación:
 *   - S_ISDIR(): verifica si es directorio
 *   - S_ISREG(): verifica si es archivo regular
 * - Constantes de permisos (bits individuales):
 *   - S_IRUSR: permiso de lectura para propietario (Read User)
 *   - S_IWUSR: permiso de escritura para propietario (Write User)
 *   - S_IXUSR: permiso de ejecución para propietario (eXecute User)
 *   Y análogos para grupo (GRP) y otros (OTH)
 */
#include <sys/stat.h>

/*
 * LIBRERÍA: <sys/types.h>
 * Define tipos de datos primitivos utilizados por llamadas al sistema.
 * Proporciona tipos fundamentales como:
 * - mode_t: representa permisos y tipo de archivo
 * - off_t: tamaños y desplazamientos en archivos
 * - time_t: representación del tiempo
 * - pid_t: identificadores de proceso
 * Esencial para portabilidad entre diferentes sistemas Unix/Linux
 */
#include <sys/types.h>

/*
 * FUNCIÓN PRINCIPAL
 * Punto de entrada del programa
 */
int main() {

    /*
     * DECLARACIÓN DE VARIABLES
     */

    // Buffer para almacenar el nombre del directorio ingresado por el usuario
    // Tamaño: 256 bytes (suficiente para la mayoría de rutas de directorios)
    char nomdir[256];

    // Buffer para almacenar la ruta completa de cada archivo/subdirectorio
    // Tamaño: 512 bytes (directorio + "/" + nombre_archivo)
    // Mayor tamaño para prevenir desbordamiento de buffer con rutas largas
    char nomfich[512];

    // Puntero a la estructura DIR que representa el directorio abierto
    // DIR es un tipo opaco (no accedemos directamente a sus miembros internos)
    DIR *d;

    // Puntero a struct dirent (read directory entry)
    // Contendrá información de cada entrada leída del directorio
    // Principalmente usamos rd1->d_name (nombre del archivo/subdirectorio)
    struct dirent *rd1;

    // Estructura stat (atributos) que almacenará metadatos completos del archivo
    // Contiene: permisos, tipo, tamaño, propietario, fechas, inodos, etc.
    struct stat atr;

    // Variable para almacenar el tiempo actual del sistema
    // time_t representa segundos transcurridos desde Unix Epoch (1 enero 1970 00:00:00 UTC)
    // Se usa para calcular si un archivo fue modificado en los últimos 10 días
    time_t fecha;

    // Variable para capturar el código de error de stat()
    // stat() retorna 0 si tiene éxito, -1 si hay error
    int er;

    /*
     * SOLICITUD Y LECTURA DEL NOMBRE DEL DIRECTORIO
     */

    // Solicitar al usuario que ingrese el nombre del directorio
    printf("Nombre directorio: ");

    // Leer el nombre del directorio de forma segura
    // fgets() es preferible a gets() porque limita caracteres leídos y previene desbordamiento
    // Parámetros: buffer destino, tamaño máximo a leer, flujo de entrada (stdin = teclado)
    fgets(nomdir, sizeof(nomdir), stdin);

    // Eliminar el carácter de nueva línea '\n' que fgets() captura automáticamente
    // strlen(nomdir)-1 apunta al último carácter (el '\n')
    // Lo reemplazamos con '\0' (terminador nulo de cadena en C)
    nomdir[strlen(nomdir)-1] = '\0';

    /*
     * OBTENCIÓN DEL TIEMPO ACTUAL DEL SISTEMA
     */

    // time(NULL) obtiene el tiempo actual del sistema como time_t
    // NULL indica que no queremos almacenar el resultado en otra variable adicional
    // Este valor se usará para calcular archivos modificados en los últimos 10 días
    // Formula: si (tiempo_actual - 10_días) < tiempo_modificación_archivo, entonces es reciente
    fecha = time(NULL);

    /*
     * INTENTO DE APERTURA DEL DIRECTORIO
     */

    // opendir() intenta abrir el directorio especificado
    // Retorna: puntero DIR* si tiene éxito, NULL si falla
    // Causas de fallo: directorio no existe, sin permisos de lectura, no es un directorio, etc.
    if ((d = opendir(nomdir)) == NULL) {
        // Si la apertura falla, informar al usuario y terminar con código de error
        printf("No existe ese directorio\n");
        return -1;  // -1 indica terminación anormal del programa

    } else {
        // El directorio se abrió exitosamente, proceder a leerlo

        /*
         * ITERACIÓN SOBRE TODAS LAS ENTRADAS DEL DIRECTORIO
         */

        // Bucle que lee cada entrada del directorio hasta el final
        // readdir() retorna:
        // - Puntero a struct dirent con la siguiente entrada si hay más entradas
        // - NULL cuando se han leído todas las entradas del directorio
        while ((rd1 = readdir(d)) != NULL) {

            /*
             * FILTRADO DE ENTRADAS ESPECIALES DEL SISTEMA
             */

            // Ignorar las entradas especiales "." y ".."
            // Estas son entradas meta-directorio presentes en todo sistema Unix/Linux:
            // ".": representa el directorio actual (self-reference)
            // "..": representa el directorio padre (parent directory)
            // strcmp() compara cadenas y retorna 0 si son idénticas
            if (strcmp(rd1->d_name, ".") == 0 || strcmp(rd1->d_name, "..") == 0)
                continue;  // Saltar a la siguiente iteración sin procesar estas entradas

            /*
             * CONSTRUCCIÓN DE LA RUTA COMPLETA (PATH ABSOLUTO)
             */

            // La función stat() necesita la ruta completa del archivo, no solo su nombre
            // Construimos: ruta_directorio + "/" + nombre_archivo

            // Paso 1: Copiar el nombre del directorio base al buffer
            strcpy(nomfich, nomdir);

            // Paso 2: Añadir el separador de directorios "/"
            // En Unix/Linux siempre es "/", en Windows sería "\\"
            strcat(nomfich, "/");

            // Paso 3: Concatenar el nombre del archivo/subdirectorio
            // Resultado final ejemplo: "/home/usuario/documentos/archivo.txt"
            strcat(nomfich, rd1->d_name);

            /*
             * MOSTRAR EL NOMBRE COMPLETO DE LA ENTRADA
             */

            // Imprimir la ruta completa del fichero o directorio
            printf("Fichero/directorio: %s\n", nomfich);

            /*
             * OBTENCIÓN DE METADATOS DEL ARCHIVO/DIRECTORIO
             */

            // stat() obtiene información completa del archivo y la almacena en 'atr'
            // Retorna: 0 si éxito, -1 si error (archivo no existe, sin permisos, etc.)
            er = stat(nomfich, &atr);

            // Si stat() falla, no podemos obtener información, saltar a siguiente entrada
            if (er != 0)
                continue;  // No procesar esta entrada y continuar con la siguiente

            /*
             * ANÁLISIS DEL MODO DEL ARCHIVO (TIPO Y PERMISOS)
             */

            // st_mode contiene información codificada en bits sobre:
            // - Tipo de archivo (bits altos): regular, directorio, enlace, etc.
            // - Permisos (bits bajos): rwx para propietario, grupo y otros
            //
            // Formato octal típico: 0100644
            // - 0100: indica archivo regular
            // - 0040: indica directorio
            // - 644: permisos (rw-r--r--)
            //
            // %o: especificador de formato para imprimir en octal (base 8)
            printf("Modo: %o\n", atr.st_mode);

            /*
             * VERIFICACIÓN DE PERMISO DE LECTURA PARA EL PROPIETARIO
             */

            // Operación AND bit a bit entre st_mode y S_IRUSR
            // S_IRUSR es una constante (máscara de bits) que representa el bit de lectura del propietario
            // Constantes de permisos en sys/stat.h:
            // - S_IRUSR (0400): Read permission, owner
            // - S_IWUSR (0200): Write permission, owner
            // - S_IXUSR (0100): Execute permission, owner
            //
            // El operador & compara bit a bit:
            // Si el bit de lectura está activado (1), la expresión es true (no cero)
            // Si el bit de lectura está desactivado (0), la expresión es false (cero)
            if (atr.st_mode & S_IRUSR)
                printf("Permiso R para propietario: Sí\n");
            else
                printf("Permiso R para propietario: No\n");

            /*
             * VERIFICACIÓN DE TIPO: ¿ES UN DIRECTORIO?
             */

            // S_ISDIR() es una macro que examina los bits de tipo en st_mode
            // Retorna valor verdadero (no cero) si es un directorio
            // Retorna 0 si es otro tipo (archivo regular, enlace simbólico, etc.)
            //
            // Otros macros similares disponibles:
            // - S_ISREG(): archivo regular
            // - S_ISLNK(): enlace simbólico
            // - S_ISCHR(): dispositivo de caracteres
            // - S_ISBLK(): dispositivo de bloques
            // - S_ISFIFO(): pipe (FIFO)
            // - S_ISSOCK(): socket
            if (S_ISDIR(atr.st_mode))
                printf("Es un directorio\n");

            /*
             * ANÁLISIS ESPECIAL PARA ARCHIVOS REGULARES MODIFICADOS RECIENTEMENTE
             */

            // Primero verificamos si es un archivo regular (no directorio, enlace, etc.)
            if (S_ISREG(atr.st_mode)) {

                /*
                 * CÁLCULO: ¿FUE MODIFICADO EN LOS ÚLTIMOS 10 DÍAS?
                 */

                // Lógica de la comparación temporal:
                // - fecha: tiempo actual en segundos desde epoch
                // - 10*24*60*60: conversión de 10 días a segundos
                //   - 10 días
                //   - 24 horas por día
                //   - 60 minutos por hora
                //   - 60 segundos por minuto
                //   - Total: 864,000 segundos
                // - (fecha - 10*24*60*60): timestamp de hace exactamente 10 días
                // - atr.st_mtime: timestamp de última modificación del archivo
                //
                // Si el archivo fue modificado DESPUÉS de hace 10 días, mostrarlo
                if ((fecha - 10*24*60*60) < atr.st_mtime) {

                    /*
                     * MOSTRAR FECHA DE MODIFICACIÓN EN FORMATO LEGIBLE
                     */

                    // ctime() convierte un time_t a cadena de texto legible
                    // Formato ejemplo: "Mon Nov 11 14:30:45 2025\n"
                    // Componentes: día_semana mes día hora:min:seg año
                    // NOTA: ctime() incluye automáticamente '\n' al final
                    //
                    // &atr.st_mtime: dirección de memoria del timestamp de modificación
                    printf("FICHERO: %s, fecha modificación: %s", rd1->d_name, ctime(&atr.st_mtime));
                }
            }

            // Línea en blanco para separar visualmente la información de cada archivo
            printf("\n");

        } // Fin del while (lectura de entradas del directorio)

        /*
         * CIERRE DEL DIRECTORIO Y LIBERACIÓN DE RECURSOS
         */

        // closedir() cierra el directorio y libera recursos del sistema operativo
        // Es crítico llamar a closedir() para evitar:
        // - Fugas de memoria (memory leaks)
        // - Agotamiento de descriptores de archivo (file descriptor exhaustion)
        // - Bloqueos de recursos que impiden acceso a otros procesos
        closedir(d);

    } // Fin del else (procesamiento del directorio)

    /*
     * FINALIZACIÓN EXITOSA DEL PROGRAMA
     */

    // Retornar 0 al sistema operativo indica terminación exitosa
    // Códigos de retorno estándar:
    // - 0: éxito
    // - No cero (1, -1, etc.): error o terminación anormal
    return 0;
}

/*****************************************************
 * Observaciones y Conclusiones:
 *
 * Programa que extiende la funcionalidad de listado
 * mostrando información detallada sobre ficheros y
 * directorios: modo, permisos, y fecha modificación si
 * es un fichero modificado en los últimos 10 días.
 *
 * Permite practicar el uso avanzado de structs como
 * stat y el manejo de formatos de fecha/hora en C.
 *
 * Restricciones: Se centra en permisos de lectura para
 * propietario y no en todos los permisos posibles.
 *
 * Posibles mejoras:
 * - Añadir visualización completa de permisos (rwx).
 * - Incorporar más filtros y opciones de listado.
 * - Modularizar funciones para mayor claridad.
 *****************************************************/

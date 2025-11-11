# Definición del compilador a usar
CC = gcc

# Opciones de compilación:
# -Wall: muestra todas las advertencias
# -Wextra: muestra advertencias adicionales
# -std=c99: estándar del lenguaje C a usar (C99)
CFLAGS = -Wall -Wextra -std=c99

# Regla por defecto: compilar ambos ejecutables
all: ficherosLab01 ficherosLab02

# Regla para compilar ficherosLab01
# Depende del archivo fuente ficherosLab01.c
ficherosLab01: ficherosLab01.c
        $(CC) $(CFLAGS) -o ficherosLab01 ficherosLab01.c

# Regla para compilar ficherosLab02
# Depende del archivo fuente ficherosLab02.c
ficherosLab02: ficherosLab02.c
        $(CC) $(CFLAGS) -o ficherosLab02 ficherosLab02.c

# Regla para limpiar los archivos ejecutables generados
clean:
        rm -f ficherosLab01 ficherosLab02

\# Makefile para el Sistema de Base de Datos de Canciones

\# Compilador y flags

CC = gcc

CFLAGS = -Wall -Wextra -g

LDFLAGS =

\# Archivos fuente

CREADOR\_SRC = creador.c

SERVER\_SRC = p2-server.c

CLIENT\_SRC = p2-client.c

\# Ejecutables

CREADOR = creador

SERVER = p2-server

CLIENT = p2-client

\# Archivos de base de datos

DATABASE = songs\_database.bin

CSV\_FILE = tracks\_features.csv

\# Target por defecto: compilar todo

all: $(CREADOR) $(SERVER) $(CLIENT)

\# Compilar el creador de base de datos

$(CREADOR): $(CREADOR\_SRC)

`	`$(CC) $(CFLAGS) $(CREADOR\_SRC) -o $(CREADOR) $(LDFLAGS)

`	`@echo "✓ Creador compilado exitosamente"

\# Compilar el servidor

$(SERVER): $(SERVER\_SRC)

`	`$(CC) $(CFLAGS) $(SERVER\_SRC) -o $(SERVER) $(LDFLAGS)

`	`@echo "✓ Servidor compilado exitosamente"

\# Compilar el cliente

$(CLIENT): $(CLIENT\_SRC)

`	`$(CC) $(CFLAGS) $(CLIENT\_SRC) -o $(CLIENT) $(LDFLAGS)

`	`@echo "✓ Cliente compilado exitosamente"

\# Crear la base de datos (requiere el archivo CSV)

database: $(CREADOR)

`	`@if [ ! -f "$(CSV\_FILE)" ]; then \

`		`echo "ERROR: No se encuentra el archivo $(CSV\_FILE)"; \

`		`echo "Por favor, coloca el archivo CSV en este directorio"; \

`		`exit 1; \

`	`fi

./$(CREADOR)

`	`@echo "✓ Base de datos creada"

\# Ejecutar el servidor (requiere base de datos)

run-server: $(SERVER)

`	`@if [ ! -f "$(DATABASE)" ]; then \

`		`echo "ERROR: No existe la base de datos. Ejecuta 'make database' primero"; \

`		`exit 1; \

`	`fi

./$(SERVER)

\# Ejecutar el cliente

run-client: $(CLIENT)

./$(CLIENT)

\# Limpiar archivos compilados

clean:

`	`rm -f $(CREADOR) $(SERVER) $(CLIENT)

`	`@echo "✓ Ejecutables eliminados"

\# Limpiar todo incluyendo la base de datos

clean-all: clean

`	`rm -f $(DATABASE)

`	`@echo "✓ Base de datos eliminada"

\# Recompilar todo desde cero

rebuild: clean all

\# Setup completo: compilar y crear base de datos

setup: all database

\# Ayuda

help:

`	`@echo "Makefile para Sistema de Base de Datos de Canciones"

`	`@echo ""

`	`@echo "Targets disponibles:"

`	`@echo "  make              - Compilar todos los programas"

`	`@echo "  make all          - Compilar todos los programas"

`	`@echo "  make creador      - Compilar solo el creador"

`	`@echo "  make p2-server    - Compilar solo el servidor"

`	`@echo "  make p2-client    - Compilar solo el cliente"

`	`@echo "  make database     - Crear la base de datos desde el CSV"

`	`@echo "  make setup        - Compilar todo y crear la base de datos"

`	`@echo "  make run-server   - Ejecutar el servidor"

`	`@echo "  make run-client   - Ejecutar el cliente"

`	`@echo "  make clean        - Eliminar ejecutables"

`	`@echo "  make clean-all    - Eliminar ejecutables y base de datos"

`	`@echo "  make rebuild      - Recompilar todo desde cero"

`	`@echo "  make help         - Mostrar esta ayuda"

`	`@echo ""

`	`@echo "Uso típico:"

`	`@echo "  1. make setup       (compilar y crear BD)"

`	`@echo "  2. make run-server  (en una terminal)"

`	`@echo "  3. make run-client  (en otra terminal)"

\# Declarar targets que no son archivos

.PHONY: all clean clean-all rebuild database run-server run-client setup help

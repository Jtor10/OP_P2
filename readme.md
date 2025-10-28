 Sistema de Base de Datos de Canciones - Cliente/Servidor
📋 Descripción del Proyecto

Sistema cliente-servidor para la gestión y búsqueda de una base de datos de canciones. El sistema utiliza una tabla hash para indexar las canciones por nombre, permitiendo búsquedas eficientes y operaciones CRUD sobre los registros musicales.
Integrantes del Proyecto

    David Nicolas Urrego Botero

    Santiago Zamora Garzón

    Jepherson Brian Torres Cruz

Arquitectura del Sistema
Componentes Principales

    creador.c - Creador de la base de datos

    p2-server.c - Servidor de base de datos

    p2_client.c - Cliente para consultas

Estructura de Archivos
text

proyecto/
├── creador.c              # Creador de la base de datos desde CSV
├── p2-server.c           # Servidor principal
├── p2_client.c           # Cliente de consultas
├── tracks_features.csv   # Dataset de canciones (debe ser proporcionado)
└── songs_database.bin    # Base de datos binaria (generada)

Funcionalidades
Servidor (p2-server.c)

    Escucha en puerto 8000 por defecto

    Manejo concurrente de múltiples clientes

    Operaciones soportadas:

        Búsquedas por nombre exacto

        Búsquedas por palabra en nombre

        Búsquedas por artista

        Búsquedas por año

        Lectura por número de registro

        Escritura de nuevos registros

Cliente (p2_client.c)

    Interfaz de menú interactiva

    Tipos de búsqueda:

        Nombre exacto

        Palabra en nombre

        Artista

        Año

    Operaciones adicionales:

        Agregar nuevas canciones

        Consulta directa por índice

Creador de Base de Datos (creador.c)

    Procesa archivo CSV tracks_features.csv

    Genera estructura hash para búsquedas eficientes

    Estadísticas de distribución de datos

    Validación y limpieza de datos

🔧 Compilación y Ejecución
Prerrequisitos

    Compilador C (gcc)

    Archivo CSV tracks_features.csv en el mismo directorio

Paso 1: Crear la Base de Datos
bash

gcc -o creador creador.c
./creador

Paso 2: Ejecutar el Servidor
bash

gcc -o servidor p2-server.c
./servidor

Paso 3: Ejecutar el Cliente
bash

gcc -o cliente p2_client.c
./cliente

Estructura de Datos
Canción (Struct Song)
c

typedef struct Song {
    char id[64];
    char name[MAX_TITLE];
    char album[MAX_ALBUM];
    char artists[MAX_ARTIST];
    int year;
    int duration_ms;
    double danceability;
    double energy;
    double tempo;
    long next;
} Song;

Tabla Hash

    Tamaño: 1000 buckets

    Función hash: DJB2 modificada

    Manejo de colisiones: Listas enlazadas

Características Técnicas
Protocolo de Comunicación

    Estructuras Request/Response para todas las operaciones

    Códigos de operación:

        1: Búsqueda

        2: Escritura de registro

        3: Lectura por índice

        4: Salida

Manejo de Datos

    Búsqueda case-insensitive

    Limpieza automática de campos

    Validación de entradas numéricas

    Formateo de duración (mm:ss)

Características Destacadas

    Eficiencia: Búsquedas en tiempo constante promedio

    Robustez: Manejo de errores y validación de datos

    Escalabilidad: Arquitectura cliente-servidor

    Persistencia: Datos almacenados en archivo binario

    Interfaz amigable: Menús intuitivos y formatos legibles

Estadísticas Generadas

El sistema proporciona estadísticas de:

    Distribución de la tabla hash

    Tasa de colisiones

    Factor de carga

    Longitud de cadenas

    Eficiencia de almacenamiento

Solución de Problemas

Error común: "No se encuentra la base de datos"

Solución: Ejecutar primero el programa creador para generar la base de datos.
Error: Archivo CSV no encontrado

Solución: Asegurarse de que tracks_features.csv esté en el directorio correcto.

Notas de Desarrollo

    Desarrollado en lenguaje C estándar

    Compatible con sistemas Unix/Linux

    Usa sockets para comunicación de red

    Implementa concurrencia básica

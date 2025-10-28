#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAX_TITLE 256
#define MAX_ARTIST 256
#define MAX_ALBUM 256
#define MAX_RESULTS 100
#define PORT 8000

// Estructuras (las mismas que el servidor)
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

typedef struct Request {
    int opcode;
    int search_type;
    char search_term[256];
    int search_year;
    int registro_numero;
    Song new_song;
} Request;

typedef struct Response {
    int status;
    int result_count;
    Song results[MAX_RESULTS];
} Response;

// Función de conexión (de tu código)
int conectar(char *hostname, int port, int debug) {
    int sockfd;
    struct hostent *he;
    struct sockaddr_in their_addr;

    if ((he = gethostbyname(hostname)) == NULL) {
        herror("Error en Nombre de Host");
        exit(1);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error en creación de socket");
        exit(1);
    }

    port = (port == 0) ? htons(PORT) : htons(port);

    their_addr.sin_family = AF_INET;
    their_addr.sin_port = port;
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    bzero(&(their_addr.sin_zero), 8);

    if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
        perror("conectar():: error tratando de conectar al server");
        exit(1);
    }

    return sockfd;
}

// Funciones auxiliares
void format_duration(int duration_ms, char *buffer, size_t buffer_size) {
    int total_seconds = duration_ms / 1000;
    int minutes = total_seconds / 60;
    int seconds = total_seconds % 60;
    snprintf(buffer, buffer_size, "%d:%02d", minutes, seconds);
}

void safe_fgets(char *buffer, size_t size) {
    if (fgets(buffer, size, stdin) == NULL) {
        buffer[0] = '\0';
    } else {
        buffer[strcspn(buffer, "\n")] = 0;
    }
}

int safe_scanf_int(const char *format, int *value) {
    int result = scanf(format, value);
    while (getchar() != '\n');
    return result;
}

double safe_scanf_double(const char *format, double *value) {
    int result = scanf(format, value);
    while (getchar() != '\n');
    return result;
}

void display_results(Response *resp) {
    if (resp->status != 0) {
        printf("Error en la operación\n");
        return;
    }

    if (resp->result_count == 0) {
        printf("NA - No se encontraron resultados\n");
        return;
    }

    printf("\n=== RESULTADOS ENCONTRADOS: %d ===\n", resp->result_count);
    
    for (int i = 0; i < resp->result_count && i < 10; i++) {
        Song *song = &resp->results[i];
        char duration_str[20];
        format_duration(song->duration_ms, duration_str, sizeof(duration_str));
        
        if (resp->result_count == 1) {
            printf("\n★ Canción encontrada ★\n");
            printf("ID: %s\n", song->id);
            printf("Nombre: %s\n", song->name);
            printf("Artista(s): %s\n", song->artists);
            printf("Álbum: %s\n", song->album);
            printf("Año: %d\n", song->year);
            printf("Duración: %s\n", duration_str);
            printf("Bailabilidad: %.3f\n", song->danceability);
            printf("Energía: %.3f\n", song->energy);
            printf("Tempo: %.1f BPM\n", song->tempo);
            printf("---\n");
        } else {
            printf("\n%d. %s - %s\n", i + 1, song->name, song->artists);
            printf("   Álbum: %s | Año: %d | Duración: %s\n", 
                   song->album, song->year, duration_str);
            if (i == 0) {
                printf("   Bailabilidad: %.3f | Energía: %.3f | Tempo: %.1f BPM\n",
                       song->danceability, song->energy, song->tempo);
            }
        }
    }
    
    if (resp->result_count > 10) {
        printf("\n... y %d resultados más\n", resp->result_count - 10);
    }
}

void display_single_song(Song *song) {
    char duration_str[20];
    format_duration(song->duration_ms, duration_str, sizeof(duration_str));
    
    printf("\n★ Canción encontrada ★\n");
    printf("ID: %s\n", song->id);
    printf("Nombre: %s\n", song->name);
    printf("Artista(s): %s\n", song->artists);
    printf("Álbum: %s\n", song->album);
    printf("Año: %d\n", song->year);
    printf("Duración: %s\n", duration_str);
    printf("Bailabilidad: %.3f\n", song->danceability);
    printf("Energía: %.3f\n", song->energy);
    printf("Tempo: %.1f BPM\n", song->tempo);
    printf("---\n");
}

void search_menu(int sockfd) {
    int option;
    char search_term[256];
    int search_year;
    
    Request req;
    Response resp;
    
    do {
        printf("\n--- BÚSQUEDA ---\n");
        printf("1. Buscar por nombre exacto\n");
        printf("2. Buscar por palabra en el nombre\n");
        printf("3. Buscar por artista\n");
        printf("4. Buscar por año\n");
        printf("5. Volver al menú principal\n");
        printf("Seleccione opción: ");
        
        if (safe_scanf_int("%d", &option) != 1) {
            printf("Entrada inválida\n");
            continue;
        }
        
        if (option == 5) break;
        
        memset(&req, 0, sizeof(Request));
        req.opcode = 1; // Búsqueda
        req.search_type = option;
        
        switch (option) {
            case 1:
                printf("Ingrese el nombre exacto de la canción: ");
                safe_fgets(search_term, sizeof(search_term));
                strcpy(req.search_term, search_term);
                break;
            case 2:
                printf("Ingrese palabra a buscar en nombres: ");
                safe_fgets(search_term, sizeof(search_term));
                strcpy(req.search_term, search_term);
                break;
            case 3:
                printf("Ingrese nombre del artista: ");
                safe_fgets(search_term, sizeof(search_term));
                strcpy(req.search_term, search_term);
                break;
            case 4:
                printf("Ingrese año a buscar: ");
                if (safe_scanf_int("%d", &search_year) == 1) {
                    req.search_year = search_year;
                } else {
                    printf("Año inválido\n");
                    continue;
                }
                break;
            default:
                printf("Opción no válida\n");
                continue;
        }
        
        // Enviar solicitud y recibir respuesta
        write(sockfd, &req, sizeof(Request));
        read(sockfd, &resp, sizeof(Response));
        
        display_results(&resp);
        
    } while (option != 5);
}

void write_record_menu(int sockfd) {
    Request req;
    Response resp;
    
    printf("\n=== AGREGAR NUEVA CANCIÓN ===\n");
    
    memset(&req, 0, sizeof(Request));
    req.opcode = 2; // Escribir registro
    
    printf("ID: ");
    safe_fgets(req.new_song.id, sizeof(req.new_song.id));
    
    printf("Nombre: ");
    safe_fgets(req.new_song.name, sizeof(req.new_song.name));
    
    printf("Álbum: ");
    safe_fgets(req.new_song.album, sizeof(req.new_song.album));
    
    printf("Artista(s): ");
    safe_fgets(req.new_song.artists, sizeof(req.new_song.artists));
    
    printf("Año: ");
    safe_scanf_int("%d", &req.new_song.year);
    
    printf("Duración (ms): ");
    safe_scanf_int("%d", &req.new_song.duration_ms);
    
    printf("Bailabilidad (0-1): ");
    safe_scanf_double("%lf", &req.new_song.danceability);
    
    printf("Energía (0-1): ");
    safe_scanf_double("%lf", &req.new_song.energy);
    
    printf("Tempo: ");
    safe_scanf_double("%lf", &req.new_song.tempo);
    
    // Enviar solicitud
    write(sockfd, &req, sizeof(Request));
    read(sockfd, &resp, sizeof(Response));
    
    if (resp.status == 0) {
        printf("¡Canción agregada exitosamente!\n");
    } else {
        printf("Error al agregar la canción\n");
    }
}

void read_by_index_menu(int sockfd) {
    int registro_numero;
    Request req;
    Response resp;
    
    printf("\n=== LEER POR NÚMERO DE REGISTRO ===\n");
    printf("Ingrese número de registro: ");
    
    if (safe_scanf_int("%d", &registro_numero) != 1) {
        printf("Número inválido\n");
        return;
    }
    
    memset(&req, 0, sizeof(Request));
    req.opcode = 3; // Leer por registro
    req.registro_numero = registro_numero;
    
    // Enviar solicitud
    write(sockfd, &req, sizeof(Request));
    read(sockfd, &resp, sizeof(Response));
    
    if (resp.status == 0 && resp.result_count > 0) {
        display_single_song(&resp.results[0]);
    } else {
        printf("NA - Registro no encontrado\n");
    }
}

int main() {
    printf("=== CLIENTE DE BASE DE DATOS DE CANCIONES ===\n");
    
    // Conectar al servidor
    int sockfd = conectar("localhost", PORT, 0);
    if (sockfd == -1) {
        fprintf(stderr, "Error conectando al servidor\n");
        return 1;
    }
    
    printf("Conectado al servidor en localhost:%d\n", PORT);
    
    int option;
    
    do {
        printf("\n=== MENÚ PRINCIPAL ===\n");
        printf("1. Realizar búsqueda\n");
        printf("2. Escribir un registro\n");
        printf("3. Leer a partir de número de registro\n");
        printf("4. Salir\n");
        printf("Seleccione una opción: ");
        
        if (safe_scanf_int("%d", &option) != 1) {
            printf("Entrada inválida\n");
            continue;
        }
        
        switch (option) {
            case 1:
                search_menu(sockfd);
                break;
            case 2:
                write_record_menu(sockfd);
                break;
            case 3:
                read_by_index_menu(sockfd);
                break;
            case 4:
                printf("Saliendo...\n");
                break;
            default:
                printf("Opción no válida\n");
        }
        
    } while (option != 4);
    
    // Enviar mensaje de salida al servidor
    Request req;
    Response resp;
    memset(&req, 0, sizeof(Request));
    req.opcode = 4; // Salir
    
    write(sockfd, &req, sizeof(Request));
    read(sockfd, &resp, sizeof(Response));
    
    close(sockfd);
    return 0;
}

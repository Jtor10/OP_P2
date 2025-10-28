#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <netdb.h>

#define HASH_SIZE 1000
#define MAX_TITLE 256
#define MAX_ARTIST 256
#define MAX_ALBUM 256
#define MAX_RESULTS 100
#define PORT 8000
#define BACKLOG 10

//Variable global para el archivo
FILE *db_file = NULL;

// Estructuras de datos
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

typedef struct HashEntry {
    long first_position;
} HashEntry;

typedef struct {
    int opcode;
    int search_type;
    char search_term[256];
    int search_year;
    int registro_numero;
    Song new_song;
} Request;

typedef struct {
    int status;
    int result_count;
    Song results[MAX_RESULTS];
} Response;

//nueva función: Inicializar base de datos
void init_database(const char *filename) {
    db_file = fopen(filename, "r+b");
    if (!db_file) {
        fprintf(stderr, "ERROR: No se puede abrir la base de datos '%s'\n", filename);
        exit(1);
    }
    printf("Base de datos abierta: %s\n", filename);
}

// Funciones de sockets (sin cambios)
int abrir_conexion(int port, int backlog, int debug) {
    int sockaux;
    int aux;
    struct sockaddr_in my_addr;

    if ((sockaux = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "Error en función socket(). Código de error %s\n", strerror(sockaux));
        return -1;
    }

    int optval = 1;
    if (setsockopt(sockaux, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        fprintf(stderr, "Error en función setsockopt()\n");
        return -1;
    }

    if (port == 0) port = PORT;

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(my_addr.sin_zero), 8);

    if ((aux = bind(sockaux, (struct sockaddr *)&my_addr, sizeof(struct sockaddr))) == -1) {
        fprintf(stderr, "Error en función bind. Código de error %s\n", strerror(aux));
        return -1;
    }

    if (backlog == 0) backlog = BACKLOG;

    if ((aux = listen(sockaux, backlog)) == -1) {
        fprintf(stderr, "Error en función listen. Código de error %s\n", strerror(aux));
        return -1;
    }

    return sockaux;
}

int aceptar_pedidos(int sockfd, int debug) {
    int newfd;
    struct sockaddr_in their_addr;
    unsigned int sin_size = sizeof(struct sockaddr_in);

    if ((newfd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1) {
        fprintf(stderr, "Error en función accept. Código de error %s\n", strerror(newfd));
        return -1;
    } else {
        if (debug)
            fprintf(stderr, "debug:: aceptar_pedidos() conexión desde: %s\n", inet_ntoa(their_addr.sin_addr));
        return newfd;
    }
}

// Hash function (sin cambios)
int hash_function(const char *name) {
    unsigned long hash = 5381;
    int c;
    
    while ((c = tolower(*name++))) {
        hash = ((hash << 5) + hash) + c;
    }
    
    return hash % HASH_SIZE;
}

// modificado: search_by_exact_name (usa db_file global)
int search_by_exact_name(const char *name, Song *results, int max_results) {
    if (!db_file) return 0;
    
    HashEntry hash_table[HASH_SIZE];
    fseek(db_file, 0, SEEK_SET);
    if (fread(hash_table, sizeof(HashEntry), HASH_SIZE, db_file) != HASH_SIZE) {
        return 0;
    }
    
    int hash_index = hash_function(name);
    long current_pos = hash_table[hash_index].first_position;
    int found = 0;
    
    while (current_pos != -1 && found < max_results) {
        fseek(db_file, current_pos, SEEK_SET);
        Song song;
        if (fread(&song, sizeof(Song), 1, db_file) != 1) break;
        
        if (strcasecmp(song.name, name) == 0) {
            results[found++] = song;
        }
        
        current_pos = song.next;
    }
    
    return found;
}

// modificado: search_by_name_word (usa db_file global)
int search_by_name_word(const char *word, Song *results, int max_results) {
    if (!db_file) return 0;
    
    HashEntry hash_table[HASH_SIZE];
    fseek(db_file, 0, SEEK_SET);
    if (fread(hash_table, sizeof(HashEntry), HASH_SIZE, db_file) != HASH_SIZE) {
        return 0;
    }
    
    int found = 0;
    char lower_word[MAX_TITLE];
    strncpy(lower_word, word, sizeof(lower_word) - 1);
    for (int i = 0; lower_word[i]; i++) {
        lower_word[i] = tolower(lower_word[i]);
    }
    
    for (int i = 0; i < HASH_SIZE && found < max_results; i++) {
        long current_pos = hash_table[i].first_position;
        
        while (current_pos != -1 && found < max_results) {
            fseek(db_file, current_pos, SEEK_SET);
            Song song;
            if (fread(&song, sizeof(Song), 1, db_file) != 1) break;
            
            char lower_name[MAX_TITLE];
            strncpy(lower_name, song.name, sizeof(lower_name) - 1);
            for (int j = 0; lower_name[j]; j++) {
                lower_name[j] = tolower(lower_name[j]);
            }
            
            if (strstr(lower_name, lower_word) != NULL) {
                results[found++] = song;
            }
            
            current_pos = song.next;
        }
    }
    
    return found;
}

// modificado: search_by_artist (usa db_file global)
int search_by_artist(const char *artist, Song *results, int max_results) {
    if (!db_file) return 0;
    
    HashEntry hash_table[HASH_SIZE];
    fseek(db_file, 0, SEEK_SET);
    if (fread(hash_table, sizeof(HashEntry), HASH_SIZE, db_file) != HASH_SIZE) {
        return 0;
    }
    
    int found = 0;
    char lower_artist[MAX_ARTIST];
    strncpy(lower_artist, artist, sizeof(lower_artist) - 1);
    for (int i = 0; lower_artist[i]; i++) {
        lower_artist[i] = tolower(lower_artist[i]);
    }
    
    for (int i = 0; i < HASH_SIZE && found < max_results; i++) {
        long current_pos = hash_table[i].first_position;
        
        while (current_pos != -1 && found < max_results) {
            fseek(db_file, current_pos, SEEK_SET);
            Song song;
            if (fread(&song, sizeof(Song), 1, db_file) != 1) break;
            
            char lower_song_artists[MAX_ARTIST];
            strncpy(lower_song_artists, song.artists, sizeof(lower_song_artists) - 1);
            for (int j = 0; lower_song_artists[j]; j++) {
                lower_song_artists[j] = tolower(lower_song_artists[j]);
            }
            
            if (strstr(lower_song_artists, lower_artist) != NULL) {
                results[found++] = song;
            }
            
            current_pos = song.next;
        }
    }
    
    return found;
}

// modificado: search_by_year (usa db_file global)
int search_by_year(int year, Song *results, int max_results) {
    if (!db_file) return 0;
    
    HashEntry hash_table[HASH_SIZE];
    fseek(db_file, 0, SEEK_SET);
    if (fread(hash_table, sizeof(HashEntry), HASH_SIZE, db_file) != HASH_SIZE) {
        return 0;
    }
    
    int found = 0;
    
    for (int i = 0; i < HASH_SIZE && found < max_results; i++) {
        long current_pos = hash_table[i].first_position;
        
        while (current_pos != -1 && found < max_results) {
            fseek(db_file, current_pos, SEEK_SET);
            Song song;
            if (fread(&song, sizeof(Song), 1, db_file) != 1) break;
            
            if (song.year == year) {
                results[found++] = song;
            }
            
            current_pos = song.next;
        }
    }
    
    return found;
}

// modificado: get_song_by_index (usa db_file global)
int get_song_by_index(int index, Song *result) {
    if (!db_file) return -1;
    
    if (index < 0) return -1;
    
    // Calcular posición directa
    long position = (HASH_SIZE * sizeof(HashEntry)) + 
                    ((long)index * sizeof(Song));
    
    if (fseek(db_file, position, SEEK_SET) != 0) {
        return -1;
    }
    
    if (fread(result, sizeof(Song), 1, db_file) != 1) {
        return -1;
    }
    
    if (result->name[0] == '\0') {
        return -1;
    }
    
    return 0;
}

// modificado: add_new_song (usa db_file global)
int add_new_song(Song *new_song) {
    if (!db_file) return -1;
    
    HashEntry hash_table[HASH_SIZE];
    fseek(db_file, 0, SEEK_SET);
    if (fread(hash_table, sizeof(HashEntry), HASH_SIZE, db_file) != HASH_SIZE) {
        return -1;
    }
    
    int hash_index = hash_function(new_song->name);
    
    fseek(db_file, 0, SEEK_END);
    long new_position = ftell(db_file);
    
    new_song->next = hash_table[hash_index].first_position;
    
    if (fwrite(new_song, sizeof(Song), 1, db_file) != 1) {
        return -1;
    }
    
    hash_table[hash_index].first_position = new_position;
    
    fseek(db_file, 0, SEEK_SET);
    if (fwrite(hash_table, sizeof(HashEntry), HASH_SIZE, db_file) != HASH_SIZE) {
        return -1;
    }
    
    // agregar: Forzar escritura inmediata
    fflush(db_file);
    fsync(fileno(db_file));
    
    return 0;
}

// modificado: handle_client (sin pasar filename)
void handle_client(int client_sock) {
    Request req;
    Response resp;
    
    while (1) {
        ssize_t bytes_read = read(client_sock, &req, sizeof(Request));
        if (bytes_read <= 0) {
            break;
        }
        
        memset(&resp, 0, sizeof(Response));
        resp.status = 0;
        
        switch (req.opcode) {
            case 1: // Búsqueda
                switch (req.search_type) {
                    case 1: // Nombre exacto
                        resp.result_count = search_by_exact_name(req.search_term, resp.results, MAX_RESULTS);
                        break;
                    case 2: // Palabra en nombre
                        resp.result_count = search_by_name_word(req.search_term, resp.results, MAX_RESULTS);
                        break;
                    case 3: // Artista
                        resp.result_count = search_by_artist(req.search_term, resp.results, MAX_RESULTS);
                        break;
                    case 4: // Año
                        resp.result_count = search_by_year(req.search_year, resp.results, MAX_RESULTS);
                        break;
                }
                break;
                
            case 2: // Escribir registro
                if (add_new_song(&req.new_song) == 0) {
                    resp.status = 0;
                    resp.result_count = 1;
                } else {
                    resp.status = -1;
                    resp.result_count = 0;
                }
                break;
                
            case 3: // Leer por número de registro
                if (get_song_by_index(req.registro_numero, &resp.results[0]) == 0) {
                    resp.result_count = 1;
                } else {
                    resp.status = -1;
                    resp.result_count = 0;
                }
                break;
                
            case 4: // Salir
                write(client_sock, &resp, sizeof(Response));
                close(client_sock);
                return;
                
            default:
                resp.status = -1;
                break;
        }
        
        write(client_sock, &resp, sizeof(Response));
    }
    
    close(client_sock);
}

// modificado main (inicializa db_file al inicio)
int main() {
    printf("=== SERVIDOR DE BASE DE DATOS DE CANCIONES ===\n");
    
    const char *bin_filename = "songs_database.bin";
    
    // Verificar existencia de la base de datos
    FILE *test_file = fopen(bin_filename, "rb");
    if (!test_file) {
        printf("ERROR: No se encuentra la base de datos '%s'\n", bin_filename);
        printf("Ejecute primero el programa creador de la base de datos.\n");
        return 1;
    }
    fclose(test_file);
    
    // agregado: Inicializar base de datos (abrir una sola vez)
    init_database(bin_filename);
    
    // Abrir conexión del servidor
    int server_sock = abrir_conexion(PORT, BACKLOG, 0);
    if (server_sock == -1) {
        fprintf(stderr, "Error iniciando servidor\n");
        if (db_file) fclose(db_file);
        return 1;
    }
    
    printf("Servidor escuchando en puerto %d...\n", PORT);
    
    // Bucle principal del servidor
    while (1) {
        int client_sock = aceptar_pedidos(server_sock, 0);
        if (client_sock == -1) {
            continue;
        }
        
        printf("Cliente conectado\n");
        handle_client(client_sock);
        printf("Cliente desconectado\n");
    }
    
    // agregado: Cerrar base de datos al terminar
    if (db_file) {
        fclose(db_file);
    }
    
    close(server_sock);
    return 0;
}

 

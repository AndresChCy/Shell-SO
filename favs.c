#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#define MAX_CMD_LEN 256
#define MAX_FAVS 100
#define MAX_HISTORY 100
#define MAX_REMOVED 100

typedef struct {
    int id;
    char command[MAX_CMD_LEN];
    bool is_favorite;
} Command;

Command history[MAX_HISTORY];                   // Historial de comandos
int history_count = 0;                          // Contador de comandos del historial
Command favorites[MAX_FAVS];                    // Lista de comandos favoritos
int fav_count = 0;                              // Contador de favoritos
char removed_cmds[MAX_REMOVED][MAX_CMD_LEN];    // Lista de comandos eliminados
int removed_count = 0;                          // Contador de comandos eliminados
char favs_file[MAX_CMD_LEN] = "";               // Ruta del archivo de favoritos (vacío inicialmente)

/*
---------------------------------------------------------------------------
                      Función para Crear el Archivo
---------------------------------------------------------------------------
*/
void create_favs_file(const char *name, const char *path) {
    char full_path[MAX_CMD_LEN];
    char name_buffer[MAX_CMD_LEN];    // Buffer temporal para manejar el nombre

    // Si no se proporciona nombre, usar "favs.txt"
    if (name == NULL || strlen(name) == 0) {
        strcpy(name_buffer, "favs.txt");
    } else {
        strncpy(name_buffer, name, sizeof(name_buffer) - 5);    // Asegurarse de que hay espacio para ".txt"
        strcat(name_buffer, ".txt");    // Añadir la extensión .txt
    }

    // Si no se proporciona ruta, usar el directorio actual
    if (path == NULL || strlen(path) == 0) {
        if (getcwd(full_path, sizeof(full_path)) == NULL) {
            perror("Error al obtener el directorio actual");
            return;
        }
        strcat(full_path, "/");
        strcat(full_path, name_buffer);
    } else {
        strcpy(full_path, path);
        strcat(full_path, "/");
        strcat(full_path, name_buffer);
    }

    // Crear y sobreescribir el archivo
    FILE *file = fopen(full_path, "w");
    if (file == NULL) {
        perror("Error al crear el archivo de favoritos");
        return;
    }
    strcpy(favs_file, full_path);
    fclose(file);
    printf("Archivo de favoritos creado: %s\n", full_path);
}

/*
---------------------------------------------------------------------------
                  Sobrescribir el archivo de favoritos
---------------------------------------------------------------------------
*/
void update_favs_file() {
    if (strlen(favs_file) == 0) {
        printf("Advertencia: No se ha cargado ningún archivo de favoritos. Use 'favs cargar' para cargar un archivo.\n");
        return;
    }

    FILE *file = fopen(favs_file, "w");
    if (file == NULL) {
        perror("Error al sobrescribir archivo de favoritos");
        return;
    }
    for (int i = 0; i < fav_count; ++i) {
        fprintf(file, "%s\n", favorites[i].command);
    }
    fclose(file);
    printf("Archivo de favoritos actualizado: %s\n", favs_file);
}

/*
---------------------------------------------------------------------------
                          Mostrar los Favoritos
---------------------------------------------------------------------------
*/
void show_favorites() {
    if (fav_count == 0) {
        printf("No hay comandos favoritos.\n");
        return;
    }
    printf("--------------------------------------------------------\n");
    printf("                   COMANDOS FAVORITOS                   \n");
    printf("--------------------------------------------------------\n");
    for (int i = 0; i < fav_count; ++i) {
        printf("%d: %s\n", favorites[i].id, favorites[i].command);
    }
    printf("--------------------------------------------------------\n");
}

/*
---------------------------------------------------------------------------
                    Agregar un Comando a los Favoritos
---------------------------------------------------------------------------
*/
void add_favorite_manual(const char *cmd) {
    for (int i = 0; i < fav_count; ++i) {
        if (strcmp(favorites[i].command, cmd) == 0) {
            printf("El comando ya está en favoritos.\n");
            return;
        }
    }
    if (fav_count < MAX_FAVS) {
        favorites[fav_count].id = fav_count + 1;
        strncpy(favorites[fav_count].command, cmd, MAX_CMD_LEN);
        favorites[fav_count].is_favorite = true;
        fav_count++;
        update_favs_file();  // Sobrescribir archivo al agregar
        printf("Comando agregado a favoritos: %s\n", cmd);
    } else {
        printf("Lista de favoritos llena.\n");
    }
}

/*
---------------------------------------------------------------------------
                   Eliminar Comando de los Favoritos
---------------------------------------------------------------------------
*/
void delete_favorite(int id) {
    for (int i = 0; i < fav_count; ++i) {
        if (favorites[i].id == id) {
            // Añadir el comando a la lista de eliminados
            strncpy(removed_cmds[removed_count], favorites[i].command, MAX_CMD_LEN);
            removed_count++;

            // Eliminar el comando de favoritos
            for (int j = i; j < fav_count - 1; ++j) {
                favorites[j] = favorites[j + 1];
            }
            fav_count--;
            update_favs_file();  // Actualizar archivo
            printf("Comando eliminado de favoritos: %d\n", id);
            return;
        }
    }
    printf("Comando no encontrado.\n");
}

/*
---------------------------------------------------------------------------
                    Cargar Comandos desde el Archivo
---------------------------------------------------------------------------
*/
void load_favs_file(const char *name, const char *path) {
    char full_path[MAX_CMD_LEN];
    char name_buffer[MAX_CMD_LEN];

    // Si no se proporciona nombre, usar "favs.txt"
    if (name == NULL || strlen(name) == 0) {
        strcpy(name_buffer, "favs.txt");
    } else {
        strncpy(name_buffer, name, sizeof(name_buffer) - 5);
        strcat(name_buffer, ".txt"); // Añadir la extensión .txt
    }

    // Si no se proporciona ruta, usar el directorio actual
    if (path == NULL || strlen(path) == 0) {
        if (getcwd(full_path, sizeof(full_path)) == NULL) {
            perror("Error al obtener el directorio actual");
            return;
        }
        strcat(full_path, "/");
        strcat(full_path, name_buffer);
    } else {
        strcpy(full_path, path);
        strcat(full_path, "/");
        strcat(full_path, name_buffer);
    }

    FILE *file = fopen(full_path, "r");
    if (file == NULL) {
        perror("Error al leer archivo de favoritos");
        return;
    }

    // Actualizar el archivo de favoritos en memoria
    strcpy(favs_file, full_path);

    // Cargar comandos del archivo
    char buffer[MAX_CMD_LEN];
    while (fgets(buffer, MAX_CMD_LEN, file)) {
        buffer[strcspn(buffer, "\n")] = 0; 
        bool is_removed = false;

        // Verificar si el comando está en la lista de eliminados
        for (int i = 0; i < removed_count; ++i) {
            if (strcmp(removed_cmds[i], buffer) == 0) {
                is_removed = true;
                break;
            }
        }

        // Agregar el comando solo si no está eliminado
        if (!is_removed) {
            add_favorite_manual(buffer);
        }
    }

    fclose(file);
    printf("Comandos cargados desde el archivo: %s\n", full_path);
}

/*
---------------------------------------------------------------------------
                        Manejo de Comandos favs
---------------------------------------------------------------------------
*/
void handle_favs_command(const char *input) {
    if (strncmp(input, "favs crear", 10) == 0) {
        char name[MAX_CMD_LEN] = "";
        char path[MAX_CMD_LEN] = "";

        sscanf(input, "favs crear %s %s", name, path);  // Leer nombre y ruta opcionales
        if (strlen(name) == 0) {
            create_favs_file(NULL, NULL);  // Crear con valores por defecto
        } else {
            create_favs_file(name, (strlen(path) == 0) ? NULL : path);  // Crear con nombre y ruta
        }
    } else if (strcmp(input, "favs mostrar") == 0) {
        show_favorites();
    } else if (strncmp(input, "favs agregar", 12) == 0) {
        char cmd[MAX_CMD_LEN];
        sscanf(input, "favs agregar %[^\n]", cmd);
        add_favorite_manual(cmd);
    } else if (strncmp(input, "favs eliminar", 13) == 0) {
        int id;
        sscanf(input, "favs eliminar %d", &id);
        delete_favorite(id);
    } else if (strncmp(input, "favs cargar", 11) == 0) {
        char name[MAX_CMD_LEN] = "";
        char path[MAX_CMD_LEN] = "";

        sscanf(input, "favs cargar %s %s", name, path);  // Leer nombre y ruta opcionales
        load_favs_file((strlen(name) == 0) ? NULL : name, (strlen(path) == 0) ? NULL : path);
    } else {
        printf("Comando desconocido.\n");
    }
}

/*
---------------------------------------------------------------------------
                                Pruebas
---------------------------------------------------------------------------
*/
int main() {
    // Simulación de entrada del usuario
    handle_favs_command("favs agregar ls");
    handle_favs_command("favs crear");
    handle_favs_command("favs agregar pwd");
    handle_favs_command("favs agregar cd");
    handle_favs_command("favs mostrar");
    handle_favs_command("favs eliminar 1");
    handle_favs_command("favs mostrar");
    return 0;
}

/*
    handle_favs_command("favs agregar ls");
    handle_favs_command("favs crear");
    handle_favs_command("favs agregar pwd");
    handle_favs_command("favs agregar cd");
    handle_favs_command("favs mostrar");
    handle_favs_command("favs eliminar 1");
    handle_favs_command("favs mostrar");
*/

/*
    handle_favs_command("favs agregar ls");
    handle_favs_command("favs cargar");
    handle_favs_command("favs mostrar");
*/

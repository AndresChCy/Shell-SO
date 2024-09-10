#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "favs.h"

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
    char name_buffer[MAX_CMD_LEN];

    // Si no se proporciona nombre, usar "favs.txt"
    if (name == NULL || strlen(name) == 0) {
        strcpy(name_buffer, "favs.txt");
    } else {
        strncpy(name_buffer, name, sizeof(name_buffer) - 5);
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
    printf("\n--------------------------------------------------------\n");
    printf("                   COMANDOS FAVORITOS                   \n");
    printf("--------------------------------------------------------\n");
    for (int i = 0; i < fav_count; ++i) {
        printf("%d: %s\n", favorites[i].id, favorites[i].command);
    }
    printf("--------------------------------------------------------\n\n");
}

/*
---------------------------------------------------------------------------
                Agregar Manualmente un Comando a los Favoritos
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
        printf("Comando agregado a favoritos: %s\n", cmd);
    } else {
        printf("Lista de favoritos llena.\n");
    }
}

/*
---------------------------------------------------------------------------
                Agregar un Comando Ejecutado a los Favoritos
---------------------------------------------------------------------------
*/
void add_executed_command_to_favorites(const char *cmd) {
    // Verificar si el comando fue previamente eliminado
    for (int i = 0; i < removed_count; ++i) {
        if (strcmp(removed_cmds[i], cmd) == 0) {
            printf("El comando '%s' fue eliminado previamente y no se agregará automáticamente.\n", cmd);
            return;
        }
    }

    // Verificar si el comando es válido (se ejecutó correctamente)
    if (system(cmd) == 0) {           // Verifica si el comando se ejecuta con éxito
        add_favorite_manual(cmd);     // Agregar el comando a la lista de favoritos
    } else {
        printf("El comando '%s' no es válido o no se pudo ejecutar.\n", cmd);
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
                favorites[j].id--;
            }
            fav_count--;
            printf("Comando eliminado de favoritos: %d\n", id);
            return;
        }
    }
    printf("Comando no encontrado.\n");
}

/*
---------------------------------------------------------------------------
                    Borrar Todos los Comando en Favs
---------------------------------------------------------------------------
*/
void clear_favorites_with_confirmation() {
    char confirmation;
    printf("¿Estás seguro de que deseas borrar todos los favoritos? (Y/N): ");
    scanf(" %c", &confirmation);

    if (confirmation == 'Y' || confirmation == 'y') {
        fav_count = 0;       // Limpiar la lista en memoria
        printf("Todos los favoritos han sido borrados.\n");
    } else {
        printf("Acción cancelada. Los favoritos no han sido borrados.\n");
    }
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
                        Búsqueda por SubString
---------------------------------------------------------------------------
*/
void search_favorites_by_substring(const char *substring) {
    int found = 0;
    for (int i = 0; i < fav_count; ++i) {
        if (strstr(favorites[i].command, substring) != NULL) {
            printf("%d: %s\n", favorites[i].id, favorites[i].command);
            found = 1;
        }
    }
    if (!found) {
        printf("No se encontraron coincidencias con '%s'.\n", substring);
    }
}

/*
---------------------------------------------------------------------------
                        Ejecutar Comando Favorito
---------------------------------------------------------------------------
*/
void execute_favorite_by_number(int num) {
    for (int i = 0; i < fav_count; ++i) {
        if (favorites[i].id == num) {
            printf("Ejecutando: %s\n", favorites[i].command);
            // ***************************************************
            // Aquí se llama a la función para ejecutar el comando
            // ***************************************************
            return;
        }
    }
    printf("No se encontró un comando con el número %d.\n", num);
}

/*
---------------------------------------------------------------------------
                         Ayuda de Comandos favs
---------------------------------------------------------------------------
*/
void show_favs_help() {
    printf("\n===========================================================================\n");
    printf("                        Ayuda de Comandos favs\n");
    printf("===========================================================================\n");
    
    printf("\nfavs crear (nombre opcional) (ruta opcional):\n");
    printf("  - Crea un archivo para almacenar la lista de comandos favoritos.\n");
    printf("  - Si no se proporciona el nombre, se usará 'favs.txt'.\n");
    printf("  - Si no se proporciona la ruta, se usará el directorio actual.\n");
    
    printf("\n---------------------------------------------------------------------------\n");

    printf("favs cargar (nombre opcional) (ruta opcional):\n");
    printf("  - Carga los comandos favoritos desde el archivo especificado.\n");
    printf("  - El archivo debe tener formato .txt.\n");
    printf("  - Si no se proporciona nombre, se usará 'favs.txt'.\n");
    printf("  - Si no se proporciona ruta, se buscará en el directorio actual.\n");

    printf("\n---------------------------------------------------------------------------\n");

    printf("favs mostrar:\n");
    printf("  - Muestra todos los comandos favoritos almacenados con su número asociado.\n");

    printf("\n---------------------------------------------------------------------------\n");

    printf("favs eliminar num1:\n");
    printf("  - Elimina un comando favorito especificados por su número.\n");
    printf("  - Nota: Los comandos eliminados ya no serán agregados automáticamente.\n");

    printf("\n---------------------------------------------------------------------------\n");

    printf("favs agregar (comando):\n");
    printf("  - Agrega manualmente un comando a la lista de favoritos.\n");
    printf("  - Ejemplo: 'favs agregar ls -la' añadirá el comando 'ls -la' como favorito.\n");

    printf("\n---------------------------------------------------------------------------\n");

    printf("favs buscar (substring):\n");
    printf("  - Busca y muestra todos los comandos favoritos que contengan la subcadena dada.\n");
    printf("  - Ejemplo: 'favs buscar ls' buscará todos los comandos que contengan 'ls'.\n");

    printf("\n---------------------------------------------------------------------------\n");

    printf("favs borrar:\n");
    printf("  - Elimina toda la lista de comandos favoritos después de pedir confirmación.\n");

    printf("\n---------------------------------------------------------------------------\n");

    printf("favs (num) ejecutar:\n");
    printf("  - Ejecuta el comando favorito especificado por su número en la lista.\n");
    printf("  - Ejemplo: 'favs 2 ejecutar' ejecutará el segundo comando en la lista de favoritos.\n");

    printf("\n---------------------------------------------------------------------------\n");

    printf("favs guardar:\n");
    printf("  - Guarda los comandos favoritos actuales en el archivo correspondiente.\n");

    printf("\n===========================================================================\n");
    printf(" Nota: Los parámetros entre paréntesis son opcionales.\n");
    printf("===========================================================================\n");
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
    } else if (strcmp(input, "favs borrar") == 0) {
        clear_favorites_with_confirmation();
    } else if (strncmp(input, "favs buscar", 11) == 0) {
        char cmd[MAX_CMD_LEN];
        sscanf(input, "favs buscar %s", cmd);
        search_favorites_by_substring(cmd);
    } else if (strncmp(input, "favs", 4) == 0 && strstr(input, "ejecutar") != NULL) {
        int num;
        sscanf(input, "favs %d ejecutar", &num);
        execute_favorite_by_number(num);
    } else if (strcmp(input, "favs guardar") == 0) {
        update_favs_file();
    } else if (strcmp(input, "favs help") == 0) {
        show_favs_help();
    } else {
        printf("Comando desconocido.\n");
    }
}

/*
---------------------------------------------------------------------------
                                Pruebas
---------------------------------------------------------------------------
*/
/*
int main() {
    char input[MAX_CMD_LEN];

    printf("\n---------------------------------------------------------------------------\n");
    printf("                         Pruebas de Comandos favs\n");
    printf("---------------------------------------------------------------------------\n");
    printf("Ingrese un comando (o 'exit' para salir):\n");

    while (1) {
        printf("> ");
        // Lee la entrada del usuario (usamos fgets para capturar la línea completa)
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break; // Si se alcanza el final de la entrada, salir
        }

        // Eliminar el salto de línea al final
        input[strcspn(input, "\n")] = 0;

        // Verificar si se quiere salir del programa
        if (strcmp(input, "exit") == 0) {
            printf("Saliendo...\n");
            break;
        }

        // Llamar a la función que maneja los comandos
        handle_favs_command(input);
    }

    return 0;
}*/

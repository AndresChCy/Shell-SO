#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#include "favs.h"
#include "shell.h"

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
extern int numPipes;                            // Declaración de numPipes

// Declaraciones de funciones de shell.c
char*** dividir_string(char *input);
void ejecutar_comandos_externos(char **parsed_str);
void pipes(int cantPipes, const char ***args);
void free_memory(char ***tokens, int numPipes);

/*
---------------------------------------------------------------------------
                       Crear Archivo de Favoritos
---------------------------------------------------------------------------
Crea un archivo para almacenar comandos favoritos. Tanto el nombre del archivo
como la ruta son opcionales. Si no se proporciona un nombre o ruta, se utiliza
'un archivo predeterminado en el directorio actual llamado "favs.txt"'.
Si solo se proporciona un nombre, se guarda en el directorio actual con ese nombre.
@param nombre El nombre del archivo (opcional)
@param ruta La ruta donde se creará el archivo (opcional)
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
                      Actualizar Archivo de Favoritos
---------------------------------------------------------------------------
Actualiza el archivo de favoritos con los comandos actualmente almacenados
en la lista de favoritos. El archivo será sobrescrito cada vez que se ejecute
esta función.
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
                        Mostrar Favoritos
---------------------------------------------------------------------------
Imprime en consola la lista de comandos favoritos actualmente almacenados.
Si no hay comandos favoritos, se informa al usuario.
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
                  Agregar Comando Manualmente a Favoritos
---------------------------------------------------------------------------
Agrega manualmente un comando a la lista de favoritos si no está presente.
@param comando El comando a agregar a la lista de favoritos
*/
void add_favorite_manual(const char *cmd) {
    for (int i = 0; i < fav_count; ++i) {
        if (strcmp(favorites[i].command, cmd) == 0) {
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
            Agregar Comando Ejecutado a la Lista de Favoritos
---------------------------------------------------------------------------
Agrega un comando ejecutado correctamente a la lista de favoritos si no
está en la lista de eliminados y no está ya presente en los favoritos.
@param comando El comando ejecutado a agregar a favoritos
*/
void add_executed_command_to_favorites(const char *cmd) {
    // Verificar si el comando fue previamente eliminado
    for (int i = 0; i < removed_count; ++i) {
        if (strcmp(removed_cmds[i], cmd) == 0) {
            printf("El comando '%s' fue eliminado previamente y no se agregará automáticamente a la lista de favoritos.\n", cmd);
            return;
        }
    }
    add_favorite_manual(cmd);
}

/*
---------------------------------------------------------------------------
                    Eliminar Comando de Favoritos
---------------------------------------------------------------------------
Elimina un comando de la lista de favoritos y lo agrega a la lista de comandos
eliminados. El comando eliminado ya no podrá volver a añadirse a favoritos
hasta que se restaure manualmente.
@param id El índice del comando a eliminar de la lista de favoritos
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
              Borrar Favoritos con Confirmación del Usuario
---------------------------------------------------------------------------
Elimina todos los comandos favoritos después de confirmar la acción con el
usuario. Los comandos eliminados no podrán ser restaurados sin cargar un nuevo
archivo de favoritos.
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
                        Cargar Archivo de Favoritos
---------------------------------------------------------------------------
Carga los comandos favoritos desde un archivo. Si el archivo contiene comandos
que no están en la lista de eliminados, se agregan a los favoritos.
Si no se proporciona nombre ni ruta, se utiliza el archivo "favs.txt" en
el directorio actual.
@param nombre El nombre del archivo a cargar (opcional)
@param ruta La ruta del archivo a cargar (opcional)
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
Busca comandos favoritos que contengan una subcadena dada. Si encuentra
alguna coincidencia, muestra los comandos en la consola, junto con su
número asociado. Si no encuentra coincidencias, notifica al usuario.
@param substring La subcadena que se buscará dentro de los comandos favoritos
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
Busca un comando en la lista de favoritos por su número asociado y lo ejecuta.
Si el número es válido, el comando se ejecuta en un proceso hijo.
@param num El número asociado al comando favorito que se desea ejecutar
*/
void execute_favorite_by_number(int num) {
    for (int i = 0; i < fav_count; ++i) {
        if (favorites[i].id == num) {
            printf("Ejecutando: %s\n", favorites[i].command);
            char inputAux[256] = "";
            strcpy(inputAux,favorites[i].command);
            char ***parsed_str = dividir_string(inputAux);
        
        if(parsed_str[0] != NULL && parsed_str[0][0] != NULL && strcmp(parsed_str[0][0], "cd") == 0){
            if (parsed_str[0][1] != NULL) {
                chdir(parsed_str[0][1]);
                numPipes = 0;
                return;
            } else {
                printf("cd: falta el argumento del directorio\n");
            }
           
        } 
       
            if (fork()==0) {
                 ejecutar_comandos_internos(*parsed_str,"");
                if (numPipes==0) {
                    ejecutar_comandos_externos(*parsed_str);
                } else {
                    pipes(numPipes, (const char ***)parsed_str);
                }
                exit(1);
            }
             
            wait(NULL);  // Esperar a que el proceso hijo termine
            free_memory(parsed_str, numPipes);
            numPipes = 0;
               
            return;
            
        }
    }
    printf("No se encontró un comando con el número %d.\n", num);
}

/*
---------------------------------------------------------------------------
                Recortar espacios al final de la cadena
---------------------------------------------------------------------------
Elimina los espacios en blanco al final de una cadena dada.
Es útil para limpiar entradas del usuario antes de procesarlas.
@param str La cadena que se va a procesar para eliminar espacios
*/
void space_killer(const char *str) {
    if (str == NULL) return;

    // Recortar espacios del final
    char *end = (char *)(str + strlen(str) - 1);;
    while (end > str && isspace((unsigned char)*end)) {
        end--;
    }
    *(end + 1) = '\0';  // Añadir terminador nulo
}

/*
---------------------------------------------------------------------------
                         Ayuda de Comandos favs
---------------------------------------------------------------------------
Muestra una lista de todos los comandos disponibles relacionados con la
gestión de favoritos y una breve descripción de cada uno.
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
    printf("===========================================================================\n");
}

/*
---------------------------------------------------------------------------
                        Manejo de Comandos favs
---------------------------------------------------------------------------
Procesa los comandos 'favs' dados por el usuario. Dependiendo del comando
específico, realiza tareas como agregar un comando favorito, eliminarlo,
buscarlo, ejecutarlo, o mostrar la lista de favoritos. También maneja la
creación, carga y guardado del archivo de favoritos.
@param input El comando completo que introduce el usuario para ser procesado
*/
void handle_favs_command(const char *input) {
    if(input != NULL) {
        space_killer(input);
    }
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


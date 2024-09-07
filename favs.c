#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CMD_LEN 256
#define MAX_FAVS 100

typedef struct {
    int id;
    char command[MAX_CMD_LEN];
} Favorite;

Favorite favorites[MAX_FAVS];       // Array para guardar los favoritos
int fav_count = 0;                  // Contador de favoritos
char favs_file[MAX_CMD_LEN];        // Ruta del archivo

/* 
--------------------------------------
    Función para Crear el Archivo
--------------------------------------
*/

void create_favs_file(const char *path) {
    FILE *file = fopen(path, "w");
    if (file == NULL) {
        perror("Error al crear el archivo de favoritos");
        return;
    }
    strcpy(favs_file, path);
    fclose(file);
    printf("Archivo de favoritos creado: %s\n", path);
}

/*
--------------------------------------
        Mostrar los Favoritos
--------------------------------------
*/

void show_favorites() {
    if (fav_count == 0) {
        printf("No hay comandos favoritos.\n");
        return;
    }
    for (int i = 0; i < fav_count; ++i) {
        printf("%d: %s\n", favorites[i].id, favorites[i].command);
    }
}

/*
--------------------------------------
  Agregar un Comando a los Favoritos
--------------------------------------
*/

void add_favorite(const char *cmd) {
    // Verificar si ya existe
    for (int i = 0; i < fav_count; ++i) {
        if (strcmp(favorites[i].command, cmd) == 0) {
            return; // Ya existe, no lo agregamos
        }
    }
    // Agregar nuevo favorito
    if (fav_count < MAX_FAVS) {
        favorites[fav_count].id = fav_count + 1;
        strncpy(favorites[fav_count].command, cmd, MAX_CMD_LEN);
        fav_count++;
    } else {
        printf("Lista de favoritos llena.\n");
    }
}

/*
--------------------------------------
    Eliminar Favoritos por Número
--------------------------------------
*/

void delete_favorite(int ids[], int num_ids) {
    for (int i = 0; i < num_ids; ++i) {
        int id_to_delete = ids[i];
        for (int j = 0; j < fav_count; ++j) {
            if (favorites[j].id == id_to_delete) {
                // Mover todos los elementos hacia la izquierda
                for (int k = j; k < fav_count - 1; ++k) {
                    favorites[k] = favorites[k + 1];
                }
                fav_count--;
                break;
            }
        }
    }
}

/*
--------------------------------------
    Buscar un Comando por Substring
--------------------------------------
*/

void search_favorites(const char *substring) {
    int found = 0;
    for (int i = 0; i < fav_count; ++i) {
        if (strstr(favorites[i].command, substring)) {
            printf("%d: %s\n", favorites[i].id, favorites[i].command);
            found = 1;
        }
    }
    if (!found) {
        printf("No se encontraron coincidencias.\n");
    }
}

/*
--------------------------------------
      Borrar Todos los Favoritos
--------------------------------------
*/

void clear_favorites() {
    fav_count = 0;
    printf("Todos los comandos favoritos fueron eliminados.\n");
}

/*
--------------------------------------
    Ejecutar un Comando por Número
--------------------------------------
*/

void execute_favorite(int id) {
    for (int i = 0; i < fav_count; ++i) {
        if (favorites[i].id == id) {
            printf("Ejecutando: %s\n", favorites[i].command);
            system(favorites[i].command); // Ejecutar el comando
            return;
        }
    }
    printf("Comando no encontrado.\n");
}

/*
--------------------------------------
   Cargar Comandos desde el Archivo
--------------------------------------
*/

void load_favorites() {
    FILE *file = fopen(favs_file, "r");
    if (file == NULL) {
        perror("Error al leer archivo de favoritos");
        return;
    }
    
    fav_count = 0;
    while (fgets(favorites[fav_count].command, MAX_CMD_LEN, file)) {
        favorites[fav_count].id = fav_count + 1;
        // Eliminar el salto de línea
        favorites[fav_count].command[strcspn(favorites[fav_count].command, "\n")] = 0;
        fav_count++;
    }
    fclose(file);
    printf("Comandos cargados desde el archivo.\n");
}

/*
--------------------------------------
    Guardar Comandos en el Archivo
--------------------------------------
*/

void save_favorites() {
    FILE *file = fopen(favs_file, "w");
    if (file == NULL) {
        perror("Error al guardar archivo de favoritos");
        return;
    }
    for (int i = 0; i < fav_count; ++i) {
        fprintf(file, "%s\n", favorites[i].command);
    }
    fclose(file);
    printf("Comandos guardados en el archivo.\n");
}
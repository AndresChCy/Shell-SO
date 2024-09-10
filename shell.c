#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include "favs.h"

#define MAX_CHAR 256

static int numPipes = 0;
int pid;

char*** dividir_string(char *input){
    char ***tokens = malloc(20 * sizeof(char**)); // alocar espacio para 10 punteros
    char *token;
    int index = 0;
    
    for (int i = 0; i < 20; i++){
        tokens[i] = malloc(20 * sizeof(char**));
        // Cada elemento apunta a NULL inicialmente 
        for (int j = 0; j < 20; j++) {
            tokens[i][j] = NULL;
        }
    }
    
    token = strtok(input, " ");
    while (token != NULL ){
        if(strcmp(token,"|") == 0){
            numPipes++;
            token = strtok(NULL, " ");
            index = 0;
            continue;
        }
        tokens[numPipes][index] = token; 
        token = strtok(NULL, " ");
        index++;
    }


    tokens[numPipes+1][index] = NULL; 
    return tokens;
}



/*
 * Función pipes para interpretar Pipes en el comando
 * cantPipes: Cantidad de pipes en el comando
 * char**** args: Matriz de strings para separar cada palabra de un comando y separar cada comando por pipes de la entrada
 */

void pipes(int cantPipes, const char ***args) {
    int status;
    int i;

    // Arreglo para usar el pipe
    int pipes[cantPipes * 2];
    // Se transforma el arreglo en pipes
    for (i = 0; i < cantPipes; i++) {
        pipe(pipes + i * 2);
    }
    // Primer proceso para servir de escritura para el siguiente proceso
    if (fork() == 0) {
        dup2(pipes[1], 1);
        // Se cierran los demás espacios del pipe ya que no se usan
        for (i = 0; i < cantPipes * 2; i++) {
            close(pipes[i]);
        }
        execvp(args[0][0], (char *const *)args[0]);
        printf("err1\n");
    }
    // Un for para los demás procesos que serán lecturas y escrituras
    for (i = 0; i < cantPipes - 1; i++) {
        if (fork() == 0) {
            dup2(pipes[i * 2], 0);
            dup2(pipes[(i * 2) + 3], 1);
            for (int j = 0; j < cantPipes * 2; j++) {
                close(pipes[j]);
            }
            execvp(args[i+1][0],(char *const *)args[i+1]);
            printf("err %i\n",i );
        }
    }
    // Fork para el último proceso del comando y que lea la última escritura
    if (fork() == 0) {
        dup2(pipes[cantPipes * 2 - 2], 0);
        for (i = 0; i < cantPipes * 2; i++) {
            close(pipes[i]);
        }
        execvp(args[cantPipes][0],(char *const *)args[cantPipes]);
        printf("err final\n");
    }

    // Padre solo espera por hijos y cierra todos los pipes porque no los usa
    for (i = 0; i < cantPipes * 2; i++) {
        close(pipes[i]);
    }
    for (i = 0; i < cantPipes + 1; i++) {
        wait(&status);
    }
}


int ejecutar_comandos_internos(char **instructions, int counter, char* input){

    if(strcmp(instructions[0], "exit") == 0){

        // Liberar la memoria tokens
        free(instructions);
        
        kill(getppid(), SIGKILL); // matar proceso padre e hijo
        kill(pid, SIGKILL);

        exit(0);

    } else if(strcmp(instructions[0],"favs")==0){
        handle_favs_command(input);
        kill(getpid(),SIGKILL);
        return 1;
    }
        
   /* for(int i = 0; i < counter; i++){
            //printf("%s ", parsed_str[i]);

        if (strcmp(instructions[i], "|") == 0){
            printf("\n pipe inserted\n");
            //run_pipe(parsed_str[i - 1], parsed_str[i + 1]); 
            return 1;  
        }
    } */

    return 0;
}

void ejecutar_comandos_externos(char **parsed_str){
    execvp(parsed_str[0], parsed_str);  // ejecuta los comandos
    printf("No se encontro el comando ingresado.\n");   
    exit(0);
}

// funcion para manejar señales
void sig_handler(int sig){
    if(sig == SIGINT){
        printf("\nSeñal SIGINT recibida. Use 'exit' para salir.\n");
        fflush(stdout);
    }
}

void free_memory(char ***tokens, int numPipes) {
    for (int i = 0; i <= numPipes; i++) {
        free(tokens[i]);  // Liberar cada array de tokens
    }
    free(tokens);  // Liberar el array principal
}

// Función mejorada para manejar fgets interrumpido por señales
int safe_fgets(char *buffer, int max_char) {
    while (1) {
        if (fgets(buffer, max_char, stdin) == NULL) {
            if (feof(stdin)) {
                clearerr(stdin);
                continue;
            } else if (errno == EINTR) {
                // Interrupción por señal, reiniciar fgets
                continue;
            } else {
                perror("Error leyendo la entrada");
                return 0;
            }
        }
        break;
    }
    return 1;
}


int main(int argc, char *argv[]) {

    char prev_command[MAX_CHAR];
    char input[MAX_CHAR];
    char inputAux[MAX_CHAR];
    char s[100];
    int count=1; 
    
    // Configurar el manejador de señales con sigaction
    struct sigaction sa;
    sa.sa_handler = sig_handler;
    sa.sa_flags = SA_RESTART; // Reinicia llamadas interrumpidas
    sigaction(SIGINT, &sa, NULL);

    system("clear");

    while(1){   
        numPipes = 0;

        printf("shell:~%s$ ", getcwd(s, 100));    // imprimir direccion de directorio
        
        if (!safe_fgets(input, MAX_CHAR)) {
            continue;
        }
        
        input[strcspn(input, "\n")] = 0;          // Eliminar el salto de linea
        
        strcpy(inputAux,input);
        char ***parsed_str;
        // comprobar si el comando es !!
        if((input[0] == 33) && (input[1] == 33)){

            parsed_str = dividir_string(prev_command); 
            if (parsed_str[0] == NULL) {
                continue;
            }

        } else {
            strcpy(prev_command, input);
            parsed_str = dividir_string(input); 
            
            if (parsed_str[0] == NULL) {
                continue;
            }
        }
            
        //identificar commandos internos
        int handled = 0; // variable para indicar si el comando ya fue manejado internamente 
        
        if(parsed_str[0] != NULL && parsed_str[0][0] != NULL && strcmp(parsed_str[0][0], "cd") == 0){
            if (parsed_str[0][1] != NULL) {
                chdir(parsed_str[0][1]);
            } else {
                printf("cd: falta el argumento del directorio\n");
            }
            handled = 1;
            numPipes = 0;
            continue;
        } 

        pid = fork();

        if (pid < 0) { // fork fallo
            printf("fork fallo\n");
            exit(1);

        } else if(pid == 0) { 
            handled = ejecutar_comandos_internos(*parsed_str, count,inputAux);
            
            if(!handled){ // ejecutar comandos externos

                if(numPipes == 0) ejecutar_comandos_externos(*parsed_str);
                else pipes(numPipes,(const char***)parsed_str);
            }
            
        } else {
            // esperar que termine de correr el comando
            if(pid != 0){wait(NULL);}
        }

        // Liberar la memoria tokens
        free_memory(parsed_str, numPipes);
        memset(input, 0, sizeof(input));


        // limpiar buffer de entrada
        
        fflush(stdin);
    }
    
    return 0;
}

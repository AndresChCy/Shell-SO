#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include "favs.h"
#include "alarma.h"
#include "shell.h"
#define MAX_CHAR 256

int numPipes = 0;
int pid;

/*
 * Función para dividir una cadena en tokens separados por espacios y pipes.
 * input: Cadena de entrada a dividir.
 * Retorna un array tridimensional de cadenas.
 */
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

/*
 * Función para ejecutar comandos internos.
 * instructions: Array de strings con el comando a ejecutar.
 * counter: Contador de comandos.
 * input: Cadena de entrada original.
 * Retorna 1 si el comando fue manejado internamente, 0 en caso contrario.
 */
int ejecutar_comandos_internos(char **instructions, char* input){

    if(strcmp(instructions[0],"favs")==0){
        handle_favs_command(input);
        kill(getpid(),SIGKILL);
        return 1;
    } else if(strcmp(instructions[0],"alarma")==0) {    
        if(instructions[1] == NULL){
            printf("Error. No se indicaron segundos.\n");
            exit(1);
        } 
        else if(instructions[2]!=NULL){
            alarma(atoi(instructions[1]),instructions[2]);
            exit(0);
        }
        else {
            alarma(atoi(instructions[1]),NULL);
            free_memory(&instructions, numPipes);
            exit(0);
        }
        kill(getpid(),SIGKILL);
        return 1 ;  
    }
    return 0;
}

/*
 * Función para ejecutar comandos externos.
 * parsed_str: Array de strings con el comando a ejecutar.
 */
void ejecutar_comandos_externos(char **parsed_str){
    execvp(parsed_str[0], parsed_str);  // ejecuta los comandos
    printf("No se encontro el comando ingresado.\n");   
    exit(1);
}

/*
 * Manejador de señales para SIGINT.
 * sig: Número de la señal recibida.
 */
void sig_handler(int sig){
    if(sig == SIGINT){
        printf("\nSeñal SIGINT recibida. Use 'exit' para salir.\n");
        fflush(stdout);
    }
}

/*
 * Función para liberar la memoria usada por los tokens.
 * tokens: Array tridimensional de tokens.
 * numPipes: Cantidad de pipes en el comando.
 */
void free_memory(char ***tokens, int numPipes) {
    for (int i = 0; i <= numPipes; i++) {
        free(tokens[i]);  // Liberar cada array de tokens
    }
    free(tokens);  // Liberar el array principal
}

/*
 * Función para manejar fgets interrumpido por señales.
 * buffer: Buffer para almacenar la cadena leída.
 * max_char: Tamaño máximo del buffer.
 * Retorna 1 si la lectura fue exitosa, 0 en caso contrario.
 */
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
    int status;
    
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
                add_executed_command_to_favorites(inputAux);
            } else {
                printf("cd: falta el argumento del directorio\n");
            }
            handled = 1;
            numPipes = 0;
            continue;
        } 
        else if(parsed_str[0]!= NULL & parsed_str[0][0] != NULL && strcmp(parsed_str[0][0],"exit")==0){
            free_memory(parsed_str,numPipes);
            exit(0);
        }
         else if(parsed_str[0]!= NULL && parsed_str[0][0] != NULL && strcmp(parsed_str[0][0],"favs")==0 ){
            if(strcmp(parsed_str[0][1],"agregar")==0){
                handle_favs_command(inputAux);
                continue ;
            }
            else if(strcmp(parsed_str[0][1],"cargar")==0){ 
                handle_favs_command(inputAux);
                continue ;}
            else if(strcmp(parsed_str[0][1],"eliminar")==0){
                handle_favs_command(inputAux);
                continue ;
            }
            else if(strcmp(parsed_str[0][1],"borrar")==0) {
                handle_favs_command(inputAux);
                continue ;
            }
            else if(parsed_str[0][2] != NULL &&strcmp(parsed_str[0][2],"ejecutar")==0){
                handle_favs_command(inputAux);
                continue ;
            }  
        }
        
        pid = fork();

        if (pid < 0) { // fork fallo
            printf("fork fallo\n");
            exit(1);

        } else if(pid == 0) { 
            handled = ejecutar_comandos_internos(*parsed_str,inputAux);
            
            if(!handled){ // ejecutar comandos externos

                if(numPipes == 0) ejecutar_comandos_externos(*parsed_str);
                else pipes(numPipes,(const char***)parsed_str);
            }
            
            exit(0); // Asegúrate de que el proceso hijo salga correctamente
        } else { // Proceso padre
            waitpid(pid, &status, 0); // Esperar a que el proceso hijo termine

            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                // Solo agregar a favoritos si el comando se ejecutó con éxito
                add_executed_command_to_favorites(inputAux);
            }
        }
        
        // Liberar la memoria tokens
        free_memory(parsed_str, numPipes);
        memset(input, 0, sizeof(input));

        // limpiar buffer de entrada
        
        fflush(stdin);

    }
    
    return 0;
}

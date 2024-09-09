#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include "favs.h"
static int numPipes = 0;
char*** dividir_string(char *input){

    char ***tokens = malloc( 20*sizeof(char**)); // alocar espacio para 10 punteros
    char *token;
    int index = 0;
    for (int i = 0; i < 20; i++){
        tokens[i] = malloc(sizeof (char*) * 20);
        // Cada elemento apunta a NULL inicialmente 
        //for (int j = 0; j < 20; j++)
          //  tokens[i][j] = NULL;
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



int pid;

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
    if(sig = SIGINT){
        kill(pid, SIGKILL);
    }
}

int main(int argc, char *argv[]) {

    #define MAX_CHAR 256
    signal(SIGINT, sig_handler); // rutina para control C
    char prev_command[MAX_CHAR];
    char input[MAX_CHAR];
    char inputAux[MAX_CHAR];
    char s[100];
    int count=1; 

    system("clear");

    while(1){   
        numPipes = 0;

        printf("shell:~%s$ ", getcwd(s, 100)); // imprimir direccion de directorio
        //fgets(input, 256, stdin);               // Leer el comando
        //input[strcspn(input, "\n")] = 0;       // Eliminar el salto de linea
        scanf("%[^\n]s", input);
        
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
        
        if(strcmp(*parsed_str[0], "cd") == 0){
            
            chdir(parsed_str[0][1]);
           // free(parsed_str);
            handled = 1;
            char ch[1];
            while(1){
                ch[0] = fgetc(stdin);
                if(ch[0] == '\n' || ch[0] == EOF) break;
            }
            numPipes = 0;
           

            for(int i = 0; i < 1000000; i++){};
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
        free(parsed_str);
        memset(input, 0, sizeof(input));


        // limpiar buffer de entrada
        
        char ch[1];
        while(1){
            ch[0] = fgetc(stdin);
            if(ch[0] == '\n' || ch[0] == EOF) break;
        }
    }   
   
    
    return 0;
}

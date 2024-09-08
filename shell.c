#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>



char** dividir_string(char *input, int *count){

    char **tokens = malloc(10 * sizeof(char*)); // alocar espacio para 10 punteros
    char *token;
    int index = 0;

    token = strtok(input, " ");
    while (token != NULL ){

        tokens[index] = token;
        token = strtok(NULL, " ");
        index++;
    }


    tokens[index] = NULL; 
    *count = index;
    return tokens;
}



/*
 * Función pipes para interpretar Pipes en el comando
 * cantPipes: Cantidad de pipes en el comando
 * char**** args: Matriz de strings para separar cada palabra de un comando y separar cada comando por pipes de la entrada
 */

void pipes(int cantPipes, const char **args[]) {
    printf("hola\n");
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
        printf("a1\n");
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
            printf("a2\n");
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
        printf("a3\n");
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


int ejecutar_comandos_internos(char **instructions, int counter){
    if(strcmp(instructions[0], "exit") == 0){
        exit(0);

    } else if (strcmp(instructions[0], "cd") == 0){
        chdir(instructions[1]);
        return 1;

    } 
    
    
    for(int i = 0; i < counter; i++){
            //printf("%s ", parsed_str[i]);

        if (strcmp(instructions[i], "|") == 0){
            printf("\n pipe inserted\n");
            //run_pipe(parsed_str[i - 1], parsed_str[i + 1]); 
            return 1;  
        }
    } 

    return 0;
}

int pid;

void ejecutar_comandos_externos(char **parsed_str){

    pid = fork();

    if (pid < 0) { // fork fallo
        printf("fork fallo\n");
        exit(1);

    } else if (pid == 0) { 
       
        execvp(parsed_str[0], parsed_str);  // ejecuta los comandos
        printf("No se encontro el comando ingresado.\n");   
    }

    // esperar que termine de correr el comando
    if(pid != 0){wait(NULL);}

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
    char s[100];
    int count; 

    system("clear");

    while(1){

        
        printf("shell:~%s$ ", getcwd(s, 100)); // imprimir direccion de directorio
        scanf("%[^\n]s", input);                // Leer el comando
        input[strcspn(input, "\n")] = 0;       // Eliminar el salto de linea

        char **parsed_str;

        // comprobar si el comando es !!
        if((input[0] == 33) && (input[1] == 33)){

            parsed_str = dividir_string(prev_command, &count); 
            if (parsed_str[0] == NULL) {
                continue;
            }

        } else {

            strcpy(prev_command, input);
            parsed_str = dividir_string(input, &count); 
            if (parsed_str[0] == NULL) {
                continue;
            }
        }

            
    
        
        //identificar commandos internos
        int handled = 0; // variable para indicar si el comando ya fue manejado internamente 

        handled = ejecutar_comandos_internos(parsed_str, count);
        if(!handled){ // ejecutar comandos
            ejecutar_comandos_externos(parsed_str);
        }
        

        // Liberar la memoria tokens
        free(parsed_str);
        memset(input, 0, sizeof(input));

        // limpiar buffer de entrada
        
        char ch[1];
        while(1){
            ch[0] = fgetc(stdin);
            if(ch[0] == '\n' || ch[0] == EOF || ch[0] == 0) break;
        }
    }   
   
    
    return 0;
}

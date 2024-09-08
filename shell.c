#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>


char *myargs[3];


void run_pipe(char arg1[], char arg2[]){  // 

    int p[2];
    pipe(p); // crea pipe (valores de descrp a partir de 3 dado 0 es entrada estandar, 1 salida, 2 error

    if (fork() == 0) {
    // hijo
        close(0); // no lee
        close(p[1]); // cierra escritura
        dup(p[0]); // copia descriptor para lectura a p[0].
        execvp(arg2[0], arg2);
    } else {
        close(1); // no escribe
        close(p[0]); // cierra lectura
        dup(p[1]); // copia descr para escritura.
        execvp(arg1[0], arg1);
    }
}

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


char**  dividir_string(char *input, int *count){
    char **tokens = malloc(10 * sizeof(char*));
    char *token;
    int index = 0;

    token = strtok(input, " ");
    while (token != NULL ){
        tokens[index] = token;
        index++;
        token = strtok(NULL, " ");
    }

    tokens[index] = NULL; 
    *count = index;
    return tokens;
}


void execute_commands(char **parsed_str){
    int pid = fork();

    if (pid < 0) { // fork fallo
        printf("fork fallo\n");
        exit(1);

    } else if (pid == 0) { //hijo
        myargs[0] = parsed_str[0];   // comando dado por linea de comando
        myargs[1] = parsed_str[1]; // argumento de comando dado en linea de comando
        myargs[3] = NULL;           // no mas argumentos para wc

        execvp(myargs[0], myargs);  // cambia imagen de hijo a wc
        printf("Aqui llegaría solo ante el error de execvp\n");   
    }

    // esperar que termine de correr el comando
    if(pid != 0){wait(NULL);}

}

int execute_inside_commands(char **instructions, int counter){
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

// funcion para manejar señales
void sig_handler(int sig){
    if(sig = SIGINT){
        exit(0);
    }
}


int main(int argc, char *argv[]) {

    signal(SIGINT, sig_handler); // rutina para control C
    //char* prev_command[5];
    char input[100];
    char s[100];
     int count; 

    while(1){


        printf("shell:~%s$ ", getcwd(s, 100)); // imprimir direccion de directorio
        fgets(input, 100, stdin);               // Leer el comando
        input[strcspn(input, "\n")] = 0       // Eliminar el salto de línea


        char **parsed_str = split_string(input, &count); 
         if (parsed_str[0] == NULL) {
            continue;
        }
    
        //char* parsed_str[5]; 
        //char* token = strtok(input, " ");
        //int counter = 0; 

        //while (token != NULL) {  // dividir input por espacios
            
         //   parsed_str[counter] = token;

            //if(strcmp(parsed_str[0], "!!") != 0){
            //    prev_command[counter] = (char*)malloc(20);
            //    strcpy(prev_command[counter], parsed_str[counter]);
            //}

            //token = strtok(NULL, " ");
            //counter++;
        //}

        //identificar commandos internos
        int handled = 0; // variable para indicar si el comando ya fue manejado internamente 


        if (strcmp(parsed_str[0], "!!") == 0){
            //make function
            for (int i = 0; i < 3; ++i){
                /* code */
                printf("%s ", prev_command[i]);
            }  

            handled = execute_inside_commands(prev_command, counter);

            if(!handled){ // ejecutar comandos
                execute_commands(parsed_str);
            }

        } else {

            handled = execute_inside_commands(parsed_str, counter);
            if(!handled){ // ejecutar comandos
                execute_commands(parsed_str);
            }
        }
        

        memset(input, 0, sizeof(input));
        // Liberar la memoria tokens
        free(parsed_str);

        // limpiar buffer de entrada
        char ch[1];
        while(1){
            ch[0] = fgetc(stdin);
            if(ch[0] == '\n' || ch[0] == EOF) break;
        }

    }   
   
    
    return 0;
}
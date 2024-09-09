#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

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

int main(){
    char *cat_args[] = {"cat", "test.txt", NULL};
    char *grep_args[] = {"grep", "hola", NULL};
    char *wc_args[] = {"wc",NULL};
    char** com[] = {cat_args,grep_args,wc_args} ;
    pipes(2,(const char***)com);
    printf("termine\n");
    return 0;
}
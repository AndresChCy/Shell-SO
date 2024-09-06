#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
/*
*Funcion pipes para interpretar Pipes en el comando
cantPipes: Cantidad de pipes en el comando
char**** args: Matriz de strings para separar cada palabra de un comando y separar cada comando por pipes de la entrada
*/

void pipes(int cantPipes, const char***args[]){
  int status;
  int i;
  //Arreglo para usar el pipe
  int pipes[cantPipes*2];
  // Se transforma el arreglo  en pipes
  for (int i = 0; i < cantPipes; i++) {
    pipe(pipes+i*2);
  }
  //Primer proceso para servir de escritura para el siguiente proceso
  if(fork()== 0){
    dup2(pipes[1], 1);
    //Se cierran los demas espacios del pipe ya que no se usan 
    for (int i = 0; i < cantPipes*2; i++){
        close(pipes[i]);
    }
    execvp(***args, **args);
  }
  //Un for para los demas procesos que seran lecturas y escrituras
  for (int i = 0; i < cantPipes; i++){
    if (fork() == 0){    
      dup2(pipes[i*2], 0);
      dup2(pipes[(i*2)+3], 1);
      for (int i = 0; i < cantPipes*2; i++){
          close(pipes[i]);
      }

      execvp(***(args +i+1), **(args+i+1));
    }
  }
  //Fork para el ultimo proceso del comando y que lea la ultima escritura
  if (fork() == 0)   {

        dup2(pipes[cantPipes*2 -1], 0);
        for (int i = 0; i < cantPipes*2; i++){
          close(pipes[i]);
      }
        execvp(***(args+cantPipes+1), **(args+cantPipes+1));
      }

      
  // padre solo espera por hijos y cierra todos los pipes porque no los usa
  
  for (int i = 0; i < cantPipes*2; i++){
          close(pipes[i]);
  }
  for (i = 0; i < cantPipes+1; i++)
    wait(&status);
}

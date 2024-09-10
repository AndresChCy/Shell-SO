#include <unistd.h>
#include<stdio.h>

/*
---------------------------------------------------------------------------
                        Función de Alarma
---------------------------------------------------------------------------
Esta función implementa una alarma que actúa como un recordatorio. Utiliza
el tiempo especificado para hacer una pausa y luego imprime un mensaje de
recordatorio.
*/
void alarma(int segs , char mensaje[]){
	printf("Alarma colocada en %d segundos!\n",segs);
	sleep(segs);
	if(mensaje != NULL){
		printf("\n%s\n",mensaje);
	}
	else{
		printf("\nAlarma suena! Ya pasaron %d segundos\n",segs);
	}
}

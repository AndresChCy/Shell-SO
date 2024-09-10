#include <unistd.h>
#include<stdio.h>

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

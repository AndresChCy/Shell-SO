#include <unistd.h>
#include<stdio.h>

void alarma(int segs , char mensaje[]){
	printf("Alarma colocada en %d segundos!\n",segs);
	sleep(segs);
	if(mensaje != NULL){
		printf("%s\n",mensaje);
	}
	else{
		printf("Alarma suena! Ya pasaron %d segundos",segs);
	}
}
int main(){
	alarma(5,NULL);
	return 0;
}
![imagen](https://github.com/user-attachments/assets/4dce8dcd-2890-47da-a9c4-8072cca22da6)# Shell-SO

Este es un programa que asimila las funciones de una terminal de Linux. La shell reconoce todos los comandos externos que reconoceria la terminal original. 

Se compila usando el comando "gcc shell.c favs.c alarma.c". (Nota: se debe tener favs.h, alarma.h y shell.h en el directorio)

---
IMPORTANTE: Para funcionamiento correcto de pipes se debe separar por espacios de los comandos.

### Comandos internos: 

 __exit__:                                    
 Cierra el programa.
  
 __!!:__                                      
 Ejecuta el comando previo.

 __cd [PATH]:__                            
 Cambia el directorio de la shell.                              


### Comandos personalizados: 

__favs__: permite mantener los comandos favoritos en forma persistente, guardados en un archivo.(En shell se puede usar favs help para instrucciones de uso)

  __favs crear ruta/nombre.txt:__           
                                          Crea archivo donde se almacenan los comandos     
                                          favoritos dada la ruta y nombre de archivo.
                                          Por defecto, cada vez que el usuario ejecuta un comando en su shell se agrega 
                                          automáticamente si y solo si no está en la lista de favoritos. Con la excepción de 
                                          los comandos asociados al manejo de favoritos.
  
  __favs mostrar:__                            
                                          Despliega la lista comandos existentes en la 
                                          lista con su respectivo número.

  __favs buscar cmd:__                      
                                          Busca comandos que contengan substring cmd en la lista de favoritos y los 
                                          despliega en pantalla junto con su número asociado.

  __favs eliminar num1,num2:__                 
                                          Eliminar comandos asociados a los números entregados entre comas.
  
  __favs borrar:__                             
                                          Borra todos los comandos en la lista de favoritos.
  
  __favs num ejecutar:__                       
                                          Ejecuta el comando, cuyo número en la lista es num. favs cargar: Lee comandos de 
                                          archivo de favoritos, los mantiene en memoria y los despliega en pantalla.
  
  __favs cargar:__                             
                                          Lee comandos de archivo de favoritos, los mantiene en memoria y los despliega en 
                                          pantalla.
                                   
                                          





__recordatorio__: Permite definir un recordatorio despues de cierta cantidad de tiempo.

__set recordatorio [SEGUNDOS] [MENSAJE]:__   
                                          Esto desplegara el mensaje ingresado en la shell al haber pasado los segundos. Sino hay mensaje se lanzara uno predeterminado







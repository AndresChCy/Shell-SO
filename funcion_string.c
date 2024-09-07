#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char** srting(char *input, int *count){
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


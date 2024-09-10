char*** dividir_string(char *input);
void pipes(int cantPipes, const char ***args);
int ejecutar_comandos_internos(char **instructions,  char* input);
void ejecutar_comandos_externos(char **parsed_str);
void free_memory(char ***tokens, int numPipes);
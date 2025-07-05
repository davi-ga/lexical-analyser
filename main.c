#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

char* read_file(char *file_path){
    FILE *pont_arq;
    char *buffer = NULL;
    long file_size;

    pont_arq = fopen(file_path,"r");
    if (pont_arq == NULL){
        printf("Erro ao tentar abrir o arquivo!");
        return NULL;
    }
    
    // Descobre o tamanho do arquivo
    fseek(pont_arq, 0, SEEK_END);
    file_size = ftell(pont_arq);
    fseek(pont_arq, 0, SEEK_SET);
    
    // Aloca memória para o buffer
    buffer = malloc(file_size + 1);
    if (buffer == NULL) {
        printf("Erro ao alocar memória!");
        fclose(pont_arq);
        return NULL;
    }
    
    // Lê o arquivo inteiro
    fread(buffer, 1, file_size, pont_arq);
    buffer[file_size] = '\0'; // Adiciona terminador de string
    
    fclose(pont_arq);
    return buffer;
}


int* ascii_tokens(char *content, int *length) {
    if (content == NULL || length == NULL) {
        return NULL;
    }
    
    // Delimitadores: espaço, parênteses, barras, chaves
    char delimiters[] = " ()\\{}";
    
    // Primeiro, conta quantos tokens existem
    int count = 0;
    char *temp = malloc(strlen(content) + 1);
    strcpy(temp, content);
    
    char *token = strtok(temp, delimiters);
    while (token != NULL) {
        count++;
        token = strtok(NULL, delimiters);
    }
    free(temp);
    
    if (count == 0) {
        *length = 0;
        return NULL;
    }
    
    // Aloca array para armazenar os códigos ASCII dos tokens
    int *tokens = malloc(count * sizeof(int));
    if (tokens == NULL) {
        *length = 0;
        return NULL;
    }
    
    // Segunda passada para extrair os tokens
    temp = malloc(strlen(content) + 1);
    strcpy(temp, content);
    
    int index = 0;
    token = strtok(temp, delimiters);
    while (token != NULL && index < count) {
        // Para cada token, vamos usar o primeiro caractere como representação
        // ou você pode modificar para calcular um hash ou outro valor
        tokens[index] = (int)token[0]; // ASCII do primeiro caractere
        index++;
        token = strtok(NULL, delimiters);
    }
    
    free(temp);
    *length = count;
    return tokens;
}



char** string_tokens(char *content, int *length) {
    if (content == NULL || length == NULL) {
        return NULL;
    }
    
    // Delimitadores: espaço, parênteses, barras, chaves
    char delimiters[] = " ()\\{};\n\r";
    
    // Primeiro, conta quantos tokens existem
    int count = 0;
    char *temp = malloc(strlen(content) + 1);
    strcpy(temp, content);
    
    char *token = strtok(temp, delimiters);
    while (token != NULL) {
        count++;
        token = strtok(NULL, delimiters);
    }
    free(temp);
    
    if (count == 0) {
        *length = 0;
        return NULL;
    }
    
    // Aloca array para armazenar ponteiros para strings
    char **tokens = malloc(count * sizeof(char*));
    if (tokens == NULL) {
        *length = 0;
        return NULL;
    }
    
    // Segunda passada para extrair os tokens
    temp = malloc(strlen(content) + 1);
    strcpy(temp, content);
    
    int index = 0;
    token = strtok(temp, delimiters);
    while (token != NULL && index < count) {
        // Aloca memória e copia o token
        tokens[index] = malloc(strlen(token) + 1);
        strcpy(tokens[index], token);
        index++;
        token = strtok(NULL, delimiters);
    }
    
    free(temp);
    *length = count;
    return tokens;
}

// ...existing code...

int main(){
    char *content = read_file("./codigos/Erro_exemplo_5.rtf");
    if (content == NULL) {
        return 1;
    }
    
    int length = 0;
    char **tokens = string_tokens(content, &length);
    
    if (tokens != NULL) {
        printf("Tokens como strings:\n");
        for (int i = 0; i < length; i++) {
            printf("tokens[%d] = \"%s\"\n", i, tokens[i]);
        }
        printf("\nTotal de tokens: %d\n", length);
        
        // Libera a memória de cada token e do array
        for (int i = 0; i < length; i++) {
            free(tokens[i]);
        }
        free(tokens);
    }
    
    free(content);
    return 0;
}
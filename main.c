#include <stdio.h>
#include <stdlib.h>

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

int* ascii_tokens(const char *content, int *length){
    
}

int main(){
    char *content = read_file("./codigos/Erro_exemplo_5.rtf");
    int i = 0;
    char token[100];

    printf("Conteúdo do arquivo:\n");
    while (content[i] != '\0') {
        printf("%d ", content[i]);  // Imprime o caractere em vez do código ASCII
        i++;
    }
    printf("\n");
    
    free(content); // Libera a memória alocada
    return 0;
}
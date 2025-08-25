// Ana Carolina dos Santos
// Davi Galdino de Oliveira

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define MAX_MEMORY (2048 * 1024) // 2048 KB em bytes

size_t memory = sizeof(memory);

// Arrays globais para evitar repetição
const char* const KEYWORDS[] = {"principal", "inteiro", "retorno", "escreva", "leia", "funcao", "senao", "se", "para"};
const int NUM_KEYWORDS = sizeof(KEYWORDS) / sizeof(KEYWORDS[0]);
const char DELIMITERS[] = " ()\\{};\n\r";

void* safe_malloc(size_t size) {
    if (memory + size > MAX_MEMORY) {
        printf("ERRO: Memória Insuficiente\n");
        exit(1);
    }
    void *ptr = malloc(size);
    if (ptr != NULL) {
        memory += size;
        // printf("Memória ocupada: %zu bytes\n", memory); 
    }
    return ptr;
}

char* clear_token(char *token) {
    if (token == NULL) return NULL;
    if (strlen(token) >= 3 &&
        (unsigned char)token[0] == 0xEF &&
        (unsigned char)token[1] == 0xBB &&
        (unsigned char)token[2] == 0xBF) {
        return token + 3; // Pula o BOM
    }
    return token;
}

char* read_file(char *file_path){
    FILE *file_ptr;
    char *buffer = NULL;
    long file_size;

    file_ptr = fopen(file_path,"r");
    if (file_ptr == NULL){
        printf("Erro ao tentar abrir o arquivo!");
        return NULL;
    }
    
    // Descobre o tamanho do arquivo
    fseek(file_ptr, 0, SEEK_END);
    file_size = ftell(file_ptr);
    fseek(file_ptr, 0, SEEK_SET);
    
    // Aloca memória para o buffer
    buffer = safe_malloc(file_size + 1);
    if (buffer == NULL) {
        printf("Erro ao alocar memória!");
        fclose(file_ptr);
        return NULL;
    }
    
    // Lê o arquivo inteiro
    fread(buffer, 1, file_size, file_ptr);
    buffer[file_size] = '\0'; // Adiciona terminador de string
    
    fclose(file_ptr);
    return buffer;
}


int* ascii_tokens(char *content, int *length) {
    if (content == NULL || length == NULL) {
        return NULL;
    }
    
    // Primeiro, conta quantos tokens existem
    int count = 0;
    char *temp = safe_malloc(strlen(content) + 1);
    strcpy(temp, content);
    
    char *token = strtok(temp, DELIMITERS);
    while (token != NULL) {
        count++;
        token = strtok(NULL, DELIMITERS);
    }
    free(temp);
    
    if (count == 0) {
        *length = 0;
        return NULL;
    }
    
    // Aloca array para armazenar os códigos ASCII dos tokens
    int *tokens = safe_malloc(count * sizeof(int));
    if (tokens == NULL) {
        *length = 0;
        return NULL;
    }
    
    // Segunda passada para extrair os tokens
    temp = safe_malloc(strlen(content) + 1);
    strcpy(temp, content);
    
    int index = 0;
    token = strtok(temp, DELIMITERS);
    while (token != NULL && index < count) {
        // Para cada token, vamos usar o primeiro caractere como representação
        tokens[index] = (int)token[0]; 
        index++;
        token = strtok(NULL, DELIMITERS);
    }
    
    free(temp);
    *length = count;
    return tokens;
};

int is_variable(const char *token) {
    // Verifica se começa com '!' e tem mais de 1 caractere
    return token && token[0] == '!' && strlen(token) > 1;
}

char** string_tokens(char *content, int *length) {
    if (content == NULL || length == NULL) {
        return NULL;
    }
    
    // Primeiro, conta quantos tokens existem
    int count = 0;
    char *temp = safe_malloc(strlen(content) + 1);
    strcpy(temp, content);
    
    char *token = strtok(temp, DELIMITERS);
    while (token != NULL) {
        count++;
        token = strtok(NULL, DELIMITERS);
    }
    free(temp);
    
    if (count == 0) {
        *length = 0;
        return NULL;
    }
    
    // Aloca array para armazenar ponteiros para strings
    char **tokens = safe_malloc(count * sizeof(char*));
    if (tokens == NULL) {
        *length = 0;
        return NULL;
    }
    
    // Segunda passada para extrair os tokens
    temp = safe_malloc(strlen(content) + 1);
    strcpy(temp, content);
    
    int index = 0;
    token = strtok(temp, DELIMITERS);
    while (token != NULL && index < count) {
        // Aloca memória e copia o token
        tokens[index] = safe_malloc(strlen(token) + 1);
        strcpy(tokens[index], token);
        index++;
        token = strtok(NULL, DELIMITERS);
    }
    
    free(temp);
    *length = count;
    return tokens;
}

int is_keyword(char *token) {
    if (token == NULL) {
        return 0;
    }
    
    // Remove BOM UTF-8 se presente no início do token
    char *cleaned = clear_token(token);
    
    // Compara o token com cada keyword
    for (int i = 0; i < NUM_KEYWORDS; i++) {
        if (strcmp(cleaned, KEYWORDS[i]) == 0) {
            return 1; // É uma keyword
        }
    }
    
    return 0; // Não é uma keyword
}

// Função para calcular a distância de Levenshtein (similaridade entre strings)
int levenshtein_distance(char *s1, char *s2) {
    int len1 = strlen(s1);
    int len2 = strlen(s2);
    
    // Matriz para programação dinâmica
    int matrix[len1 + 1][len2 + 1];
    
    // Inicializa primeira linha e coluna
    for (int i = 0; i <= len1; i++) {
        matrix[i][0] = i;
    }
    for (int j = 0; j <= len2; j++) {
        matrix[0][j] = j;
    }
    
    // Preenche a matriz
    for (int i = 1; i <= len1; i++) {
        for (int j = 1; j <= len2; j++) {
            int cost = (s1[i-1] == s2[j-1]) ? 0 : 1;
            
            int min_val = matrix[i-1][j] + 1;     // Deleção
            int temp = matrix[i][j-1] + 1;        // Inserção
            if (temp < min_val) min_val = temp;
            
            temp = matrix[i-1][j-1] + cost;       // Substituição
            if (temp < min_val) min_val = temp;
            
            matrix[i][j] = min_val;
        }
    }
    
    return matrix[len1][len2];
}

int has_lexical_error(char *token) {
    if (token == NULL) {
        return 0;
    }
    
    // Se já é uma keyword correta, não é erro
    if (is_keyword(token)) {
        return 0;
    }
    
    // Remove BOM UTF-8 se presente no início do token
    char *cleaned = clear_token(token);
    
    // Se o token limpo está vazio, não é erro léxico
    if (strlen(cleaned) == 0) {
        return 0;
    }
    
    // Converte o token para minúsculas para comparação
    char *token_lower = safe_malloc(strlen(cleaned) + 1);
    strcpy(token_lower, cleaned);
    for (int i = 0; token_lower[i]; i++) {
        token_lower[i] = tolower(token_lower[i]);
    }
    
    // Verifica similaridade com cada keyword
    for (int i = 0; i < NUM_KEYWORDS; i++) {
        int distance = levenshtein_distance(token_lower, (char*)KEYWORDS[i]);
        int keyword_len = strlen(KEYWORDS[i]);
        int token_len = strlen(token_lower);
        
        // Se a distância é pequena em relação ao tamanho da palavra (erro léxico)
        if (distance > 0 && distance <= 2 && keyword_len > 3) {
            free(token_lower);
            return 1; // Erro léxico detectado
        }
        
        // Se é um prefixo muito próximo da keyword (como "escrev" para "escreva")
        if (strncmp(token_lower, KEYWORDS[i], token_len) == 0 && 
            token_len >= keyword_len - 2 && token_len < keyword_len) {
            free(token_lower);
            return 1; // Erro léxico detectado
        }
    }
    
    free(token_lower);
    return 0; // Não é erro léxico
}

// Função para sugerir a keyword mais próxima
char* suggest_keyword(char *token) {
    if (token == NULL) {
        return NULL;
    }
    
    // Remove BOM UTF-8 se presente no início do token
    char *cleaned = clear_token(token);
    
    // Se o token limpo está vazio, não há sugestão
    if (strlen(cleaned) == 0) {
        return NULL;
    }
    
    // Converte o token para minúsculas para comparação
    char *token_lower = safe_malloc(strlen(cleaned) + 1);
    strcpy(token_lower, cleaned);
    for (int i = 0; token_lower[i]; i++) {
        token_lower[i] = tolower(token_lower[i]);
    }
    
    int min_distance = 999;
    char *best_match = NULL;
    
    // Encontra a keyword com menor distância
    for (int i = 0; i < NUM_KEYWORDS; i++) {
        int distance = levenshtein_distance(token_lower, (char*)KEYWORDS[i]);
        int keyword_len = strlen(KEYWORDS[i]);
        int token_len = strlen(token_lower);
        
        // Considera como candidato se:
        // 1. A distância é pequena (1-2 caracteres de diferença)
        // 2. É um prefixo próximo da keyword
        if ((distance > 0 && distance <= 2 && keyword_len > 3) ||
            (strncmp(token_lower, KEYWORDS[i], token_len) == 0 && 
             token_len >= keyword_len - 2 && token_len < keyword_len)) {
            
            if (distance < min_distance) {
                min_distance = distance;
                best_match = (char*)KEYWORDS[i];
            }
        }
    }
    
    free(token_lower);
    return best_match;
}

int main() {
    DIR *dir;
    struct dirent *entry;
    char path[512];

    dir = opendir("./data");
    if (dir == NULL) {
        printf("Não foi possível abrir o diretório ./data\n");
        return 1;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(path, sizeof(path), "./data/%s", entry->d_name);

        printf("\n==============================\n");
        printf("Processando arquivo: %s\n", path);

        char *content = read_file(path);
        if (content == NULL) {
            printf("Erro ao ler o arquivo: %s\n", path);
            continue;
        }

        int length = 0;
        char **tokens = string_tokens(content, &length);

        if (tokens != NULL) {    
            printf("\nTotal de tokens: %d\n", length);
            
            printf("\nClassificação dos tokens:\n");
            int i = 0;
            while (i < length) {
                char *cleaned = clear_token(tokens[i]);
                if (strncmp(tokens[i], "“", strlen("“")) == 0) {
                    printf("tokens[%d] = \"%s\" -> STRING\n", i, tokens[i]);
                    i++;
                    while (i < length) {
                        size_t len = strlen(tokens[i]);
                        if (len >= strlen("”") && strcmp(&tokens[i][len - strlen("”")], "”") == 0) {
                            printf("tokens[%d] = \"%s\" -> STRING\n", i, tokens[i]);
                            break;
                        } else {
                            printf("tokens[%d] = \"%s\" -> STRING\n", i, tokens[i]);
                            i++;
                        }
                    }
                } else if (strcmp(cleaned, "funcao") == 0) {
                    if (i + 1 < length && strncmp(tokens[i + 1], "__", 2) == 0) {
                        printf("tokens[%d] = \"%s\" -> KEYWORD\n", i, tokens[i]);
                        printf("tokens[%d] = \"%s\" -> FUNC_NAME\n", i + 1, tokens[i + 1]);
                        i += 2; 
                        continue;
                    } else if (i + 1 < length) {
                        printf("tokens[%d] = \"%s\" -> KEYWORD\n", i, tokens[i]);
                        char *suggestion = suggest_keyword(tokens[i + 1]);
                        if (suggestion != NULL) {
                            printf("tokens[%d] = \"%s\" -> LEXICAL ERROR (Você quis dizer '%s'?)\n", i + 1, tokens[i + 1], suggestion);
                        } else {
                            printf("tokens[%d] = \"%s\" -> LEXICAL ERROR\n", i + 1, tokens[i + 1]);
                        }
                        printf("ERRO ENCONTRADO: Finalizando a análise.\n");
                        break; 
                    } else {
                        printf("tokens[%d] = \"%s\" -> KEYWORD\n", i, tokens[i]);
                        printf("tokens[%d] = <FIM> -> LEXICAL ERROR\n", i + 1);
                        printf("ERRO ENCONTRADO: Finalizando a análise.\n");
                        break;
                    }
                } else if ((i + 1 < length) && strcmp(tokens[i + 1], "=") == 0) {
                    if (tokens[i][0] != '!') {
                        char *suggestion = suggest_keyword(tokens[i]);
                        if (suggestion != NULL) {
                            printf("tokens[%d] = \"%s\" -> LEXICAL ERROR (Você quis dizer '%s'?)\n", i, tokens[i], suggestion);
                        } else {
                            printf("tokens[%d] = \"%s\" -> LEXICAL ERROR\n", i, tokens[i]);
                        }
                        printf("ERRO ENCONTRADO: Finalizando a análise.\n");
                        break;
                    }
                } else if (strcmp(tokens[i], ";") == 0) {
                    printf("tokens[%d] = \"%s\" -> SEMICOLON\n", i, tokens[i]);
                } else if (strcmp(tokens[i], "(") == 0) {
                    printf("tokens[%d] = \"%s\" -> LEFT_PAREN\n", i, tokens[i]);
                } else if (strcmp(tokens[i], ")") == 0) {
                    printf("tokens[%d] = \"%s\" -> RIGHT_PAREN\n", i, tokens[i]);
                } else if (strcmp(tokens[i], "{") == 0) {
                    printf("tokens[%d] = \"%s\" -> LEFT_BRACE\n", i, tokens[i]);
                } else if (strcmp(tokens[i], "}") == 0) {
                    printf("tokens[%d] = \"%s\" -> RIGHT_BRACE\n", i, tokens[i]);
                } else if (isdigit(tokens[i][0])) {
                    printf("tokens[%d] = \"%s\" -> INTEGER\n", i, tokens[i]);
                } else if (is_variable(tokens[i])) {
                    printf("tokens[%d] = \"%s\" -> VARIABLE\n", i, tokens[i]);
                } else if (has_lexical_error(tokens[i])) {
                    char *suggestion = suggest_keyword(tokens[i]);
                    if (suggestion != NULL) {
                        printf("tokens[%d] = \"%s\" -> LEXICAL ERROR (Você quis dizer '%s'?)\n", i, tokens[i], suggestion);
                    } else {
                        printf("tokens[%d] = \"%s\" -> LEXICAL ERROR\n", i, tokens[i]);
                    }
                    printf("ERRO ENCONTRADO: Finalizando a análise.\n");
                    break;
                } else if (is_keyword(tokens[i])) {
                    printf("tokens[%d] = \"%s\" -> KEYWORD\n", i, tokens[i]);
                }else {
                    printf("tokens[%d] = \"%s\" -> IDENTIFIER/OTHER\n", i, tokens[i]);
                }
                i++;
            }

            for (int j = 0; j < length; j++) {
                free(tokens[j]);
            }
            free(tokens);
        }
        printf("\nMemória ocupada: %zu Bytes ou %.2f KB\n", memory, memory / 1024.0);
        free(content);
    }

    closedir(dir);
    return 0;
}
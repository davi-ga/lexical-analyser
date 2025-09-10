// Ana Carolina dos Santos
// Davi Galdino de Oliveira

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#define MAX_MEMORY (2048 * 1024) // 2048 KB em bytes
#define MAX_SYMBOLS 1000

size_t memory = sizeof(memory);

// Estrutura para a tabela de símbolos
typedef enum {
    SYMBOL_VARIABLE,
    SYMBOL_FUNCTION,
    SYMBOL_PARAMETER
} SymbolType;

typedef enum {
    TYPE_INTEGER,
    TYPE_STRING,
    TYPE_FLOAT,
    TYPE_VOID,
    TYPE_UNKNOWN
} DataType;

typedef struct Symbol {
    char *name;
    SymbolType symbol_type;
    DataType data_type;
    int scope_level;
    int line_declared;
    bool is_used;
    int param_count;  
    char **param_names;
    DataType *param_types; 
    struct Symbol *next;
} Symbol;

typedef struct SymbolTable {
    Symbol *symbols[MAX_SYMBOLS];
    int count;
    int current_scope;
} SymbolTable;

// Variável global da tabela de símbolos
SymbolTable symbol_table = {0};

// Declarações de função
void* safe_malloc(size_t size);
Symbol* lookup_symbol_current_scope(const char *name);
int is_variable(const char *token);

// Funções da tabela de símbolos
unsigned int hash_function(const char *name) {
    unsigned int hash = 0;
    for (int i = 0; name[i] != '\0'; i++) {
        hash = hash * 31 + name[i];
    }
    return hash % MAX_SYMBOLS;
}

void init_symbol_table() {
    symbol_table.count = 0;
    symbol_table.current_scope = 0;
    for (int i = 0; i < MAX_SYMBOLS; i++) {
        symbol_table.symbols[i] = NULL;
    }
}

Symbol* create_symbol(const char *name, SymbolType symbol_type, DataType data_type, int line) {
    Symbol *new_symbol = safe_malloc(sizeof(Symbol));
    new_symbol->name = safe_malloc(strlen(name) + 1);
    strcpy(new_symbol->name, name);
    new_symbol->symbol_type = symbol_type;
    new_symbol->data_type = data_type;
    new_symbol->scope_level = symbol_table.current_scope;
    new_symbol->line_declared = line;
    new_symbol->is_used = false;
    new_symbol->param_count = 0;
    new_symbol->param_names = NULL;
    new_symbol->param_types = NULL;
    new_symbol->next = NULL;
    return new_symbol;
}

bool add_symbol(const char *name, SymbolType symbol_type, DataType data_type, int line) {
    // Verifica se o símbolo já existe no escopo atual
    Symbol *existing = lookup_symbol_current_scope(name);
    if (existing != NULL) {
        printf("SEMANTIC ERROR: Símbolo '%s' já declarado na linha %d\n", name, existing->line_declared);
        return false;
    }
    
    unsigned int index = hash_function(name);
    Symbol *new_symbol = create_symbol(name, symbol_type, data_type, line);
    
    // Inserção no início da lista ligada (tratamento de colisão)
    new_symbol->next = symbol_table.symbols[index];
    symbol_table.symbols[index] = new_symbol;
    symbol_table.count++;
    
    return true;
}

Symbol* lookup_symbol(const char *name) {
    unsigned int index = hash_function(name);
    Symbol *current = symbol_table.symbols[index];
    
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            current->is_used = true;
            return current;
        }
        current = current->next;
    }
    return NULL;
}

Symbol* lookup_symbol_current_scope(const char *name) {
    unsigned int index = hash_function(name);
    Symbol *current = symbol_table.symbols[index];
    
    while (current != NULL) {
        if (strcmp(current->name, name) == 0 && 
            current->scope_level == symbol_table.current_scope) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void enter_scope() {
    symbol_table.current_scope++;
}

void exit_scope() {
    // Remove símbolos do escopo atual
    for (int i = 0; i < MAX_SYMBOLS; i++) {
        Symbol *current = symbol_table.symbols[i];
        Symbol *prev = NULL;
        
        while (current != NULL) {
            if (current->scope_level == symbol_table.current_scope) {
                if (prev == NULL) {
                    symbol_table.symbols[i] = current->next;
                } else {
                    prev->next = current->next;
                }
                
                if (!current->is_used) {
                    printf("WARNING: Símbolo '%s' declarado mas não utilizado (linha %d)\n", 
                           current->name, current->line_declared);
                }
                
                free(current->name);
                Symbol *to_delete = current;
                current = current->next;
                free(to_delete);
                symbol_table.count--;
            } else {
                prev = current;
                current = current->next;
            }
        }
    }
    
    if (symbol_table.current_scope > 0) {
        symbol_table.current_scope--;
    }
}

const char* symbol_type_to_string(SymbolType type) {
    switch (type) {
        case SYMBOL_VARIABLE: return "VARIABLE";
        case SYMBOL_FUNCTION: return "FUNCTION";
        case SYMBOL_PARAMETER: return "PARAMETER";
        default: return "UNKNOWN";
    }
}

const char* data_type_to_string(DataType type) {
    switch (type) {
        case TYPE_INTEGER: return "INTEGER";
        case TYPE_STRING: return "STRING";
        case TYPE_FLOAT: return "FLOAT";
        case TYPE_VOID: return "VOID";
        default: return "UNKNOWN";
    }
}

void infer_parameter_types() {
    // Percorre a tabela de símbolos para inferir tipos de parâmetros baseado no uso
    for (int i = 0; i < MAX_SYMBOLS; i++) {
        Symbol *current = symbol_table.symbols[i];
        while (current != NULL) {
            if (current->symbol_type == SYMBOL_PARAMETER && current->data_type == TYPE_UNKNOWN) {
                // Por padrão, assume que parâmetros são inteiros se usados em operações aritméticas
                // Esta é uma simplificação - em um compilador real, faria análise mais sofisticada
                current->data_type = TYPE_INTEGER;
            }
            current = current->next;
        }
    }
}

void update_parameter_type(const char *param_name, DataType new_type) {
    Symbol *param = lookup_symbol(param_name);
    if (param != NULL && param->symbol_type == SYMBOL_PARAMETER) {
        param->data_type = new_type;
    }
}

void add_function_parameter(const char *func_name, const char *param_name, DataType param_type) {
    Symbol *func = lookup_symbol(func_name);
    if (func != NULL && func->symbol_type == SYMBOL_FUNCTION) {
        func->param_count++;
        
        // Realoca arrays para acomodar novo parâmetro
        func->param_names = realloc(func->param_names, func->param_count * sizeof(char*));
        func->param_types = realloc(func->param_types, func->param_count * sizeof(DataType));
        
        // Adiciona o novo parâmetro
        func->param_names[func->param_count - 1] = safe_malloc(strlen(param_name) + 1);
        strcpy(func->param_names[func->param_count - 1], param_name);
        func->param_types[func->param_count - 1] = param_type;
    }
}

bool validate_function_call(const char *func_name, int provided_params, int line) {
    Symbol *func = lookup_symbol(func_name);
    if (func == NULL) {
        printf("SEMANTIC ERROR (linha %d): Função '%s' não declarada\n", line, func_name);
        return false;
    }
    
    if (func->symbol_type != SYMBOL_FUNCTION) {
        printf("SEMANTIC ERROR (linha %d): '%s' não é uma função\n", line, func_name);
        return false;
    }
    
    if (func->param_count != provided_params) {
        printf("SEMANTIC ERROR (linha %d): Função '%s' espera %d parâmetros, mas %d foram fornecidos\n", 
               line, func_name, func->param_count, provided_params);
        return false;
    }
    
    func->is_used = true;
    return true;
}

bool validate_function_declaration(const char *func_name) {
    // Verifica se o nome da função segue o padrão (deve começar com __)
    if (strncmp(func_name, "__", 2) != 0) {
        printf("SEMANTIC ERROR: Nome de função '%s' deve começar com '__'\n", func_name);
        return false;
    }
    
    // Verifica se não é apenas "__"
    if (strlen(func_name) <= 2) {
        printf("SEMANTIC ERROR: Nome de função '%s' é inválido (muito curto)\n", func_name);
        return false;
    }
    
    return true;
}

bool is_parameter_redeclaration(const char *var_name) {
    // Verifica se a variável é um parâmetro no escopo atual
    Symbol *existing = lookup_symbol(var_name);
    if (existing != NULL && existing->symbol_type == SYMBOL_PARAMETER && 
        existing->scope_level == symbol_table.current_scope) {
        return true;
    }
    return false;
}

bool validate_parameter_list(char **tokens, int start_idx, int end_idx) {
    // Valida se os parâmetros estão corretamente separados por vírgulas
    bool expecting_param = true;
    bool expecting_comma = false;
    
    for (int i = start_idx; i <= end_idx; i++) {
        if (expecting_param) {
            if (is_variable(tokens[i])) {
                expecting_param = false;
                expecting_comma = true;
            } else {
                printf("SYNTAX ERROR: Esperado parâmetro na posição %d, encontrado '%s'\n", i, tokens[i]);
                return false;
            }
        } else if (expecting_comma) {
            if (strcmp(tokens[i], ",") == 0) {
                expecting_param = true;
                expecting_comma = false;
            } else if (strcmp(tokens[i], ")") == 0) {
                break; // Final da lista de parâmetros
            } else {
                printf("SYNTAX ERROR: Esperada vírgula após parâmetro, encontrado '%s'\n", tokens[i]);
                return false;
            }
        }
    }
    
    if (expecting_param) {
        printf("SYNTAX ERROR: Lista de parâmetros incompleta - esperado parâmetro após vírgula\n");
        return false;
    }
    
    return true;
}

bool validate_leia_command(char **tokens, int start_idx, int *end_idx, int current_line) {
    // 4.3. Haverá sempre um duplo balanceamento utilizando os parênteses
    if (start_idx >= *end_idx || strcmp(tokens[start_idx], "(") != 0) {
        printf("SYNTAX ERROR (linha %d): Comando 'leia' deve ser seguido por '('\n", current_line);
        return false;
    }
    
    int i = start_idx + 1;
    int paren_count = 1;
    int close_paren_pos = -1;
    bool expecting_variable = true;
    bool expecting_comma = false;
    int var_count = 0;
    
    // Procura o fechamento dos parênteses e valida o conteúdo
    while (i < *end_idx && paren_count > 0) {
        if (strcmp(tokens[i], "(") == 0) {
            paren_count++;
        } else if (strcmp(tokens[i], ")") == 0) {
            paren_count--;
            if (paren_count == 0) {
                close_paren_pos = i;
                break;
            }
        } else if (paren_count == 1) { // Apenas no nível principal dos parênteses
            if (expecting_variable) {
                if (is_variable(tokens[i])) {
                    // 4.1. Variáveis devem ser declaradas anteriormente
                    char *var_name = safe_malloc(strlen(tokens[i]) + 1);
                    strcpy(var_name, tokens[i]);
                    char *comma = strchr(var_name, ',');
                    if (comma) *comma = '\0';
                    
                    Symbol *var = lookup_symbol(var_name);
                    if (var == NULL) {
                        printf("SEMANTIC ERROR (linha %d): Variável '%s' não foi declarada\n", current_line, var_name);
                        free(var_name);
                        return false;
                    }
                    
                    // 4.2. Não podem ser feitas declarações dentro da estrutura de leitura
                    if (var->symbol_type != SYMBOL_VARIABLE && var->symbol_type != SYMBOL_PARAMETER) {
                        printf("SEMANTIC ERROR (linha %d): '%s' não é uma variável válida para leitura\n", current_line, var_name);
                        free(var_name);
                        return false;
                    }
                    
                    var->is_used = true;
                    var_count++;
                    expecting_variable = false;
                    expecting_comma = true;
                    free(var_name);
                } else {
                    printf("SYNTAX ERROR (linha %d): Esperada variável no comando 'leia', encontrado '%s'\n", current_line, tokens[i]);
                    return false;
                }
            } else if (expecting_comma) {
                if (strcmp(tokens[i], ",") == 0) {
                    expecting_variable = true;
                    expecting_comma = false;
                } else {
                    printf("SYNTAX ERROR (linha %d): Esperada vírgula entre variáveis no comando 'leia', encontrado '%s'\n", current_line, tokens[i]);
                    return false;
                }
            }
        }
        i++;
    }
    
    if (close_paren_pos == -1) {
        printf("SYNTAX ERROR (linha %d): Comando 'leia' sem fechamento de parênteses\n", current_line);
        return false;
    }
    
    if (expecting_variable && var_count > 0) {
        printf("SYNTAX ERROR (linha %d): Comando 'leia' termina com vírgula sem variável\n", current_line);
        return false;
    }
    
    if (var_count == 0) {
        printf("SYNTAX ERROR (linha %d): Comando 'leia' deve ter pelo menos uma variável\n", current_line);
        return false;
    }
    
    // 4.4. A linha deve ser finalizada com ponto e vírgula
    if (close_paren_pos + 1 < *end_idx && strcmp(tokens[close_paren_pos + 1], ";") != 0) {
        printf("SYNTAX ERROR (linha %d): Comando 'leia' deve ser finalizado com ';'\n", current_line);
        return false;
    }
    
    *end_idx = close_paren_pos + 1; // Atualiza a posição final
    return true;
}

void print_symbol_table() {
    printf("\n======== TABELA DE SÍMBOLOS ========\n");
    printf("%-20s %-12s %-10s %-8s %-8s %-8s %-15s\n", 
           "NOME", "TIPO_SIMBOLO", "TIPO_DADO", "ESCOPO", "LINHA", "USADO", "PARÂMETROS");
    printf("--------------------------------------------------------------------------------\n");
    
    for (int i = 0; i < MAX_SYMBOLS; i++) {
        Symbol *current = symbol_table.symbols[i];
        while (current != NULL) {
            char param_info[100] = "";
            
            if (current->symbol_type == SYMBOL_FUNCTION && current->param_count > 0) {
                snprintf(param_info, sizeof(param_info), "(%d params)", current->param_count);
            }
            
            printf("%-20s %-12s %-10s %-8d %-8d %-8s %-15s\n",
                   current->name,
                   symbol_type_to_string(current->symbol_type),
                   data_type_to_string(current->data_type),
                   current->scope_level,
                   current->line_declared,
                   current->is_used ? "SIM" : "NAO",
                   param_info);
            current = current->next;
        }
    }
    printf("====================================\n");
    printf("Total de símbolos: %d\n\n", symbol_table.count);
}

DataType string_to_data_type(const char *type_str) {
    if (strcmp(type_str, "inteiro") == 0) return TYPE_INTEGER;
    if (strcmp(type_str, "texto") == 0) return TYPE_STRING;
    if (strcmp(type_str, "decimal") == 0) return TYPE_FLOAT;
    return TYPE_UNKNOWN;
}

// Arrays globais para evitar repetição
const char* const KEYWORDS[] = {"principal", "inteiro", "retorno", "escreva", "leia", "funcao", "senao", "se", "para"};
const int NUM_KEYWORDS = sizeof(KEYWORDS) / sizeof(KEYWORDS[0]);
// const char DELIMITERS[] = " ()\\{};\n\r";
const char DELIMITERS[] = " ";
const char SPECIAL_TOKENS[] = "()\\{};\n\r\"";
const char* MULTI_TOKENS[] = {"==", "<=", ">=", "&&", "||", "<>"};
#define NUM_MULTI_TOKENS (sizeof(MULTI_TOKENS) / sizeof(MULTI_TOKENS[0]))
bool PRINCIPAL_FUNC = false;

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

int is_variable(const char *token) {
    // Verifica se começa com '!' e tem mais de 1 caractere
    return token && token[0] == '!' && strlen(token) > 1;
}
char** string_tokens(char *content, int *length) {
    if (content == NULL || length == NULL) return NULL;

    int count = 0;
    int i = 0;

    // contar tokens
    while (content[i] != '\0') {
        int matched = 0;
        for (int k = 0; k < NUM_MULTI_TOKENS; k++) {
            int len = strlen(MULTI_TOKENS[k]);
            if (strncmp(&content[i], MULTI_TOKENS[k], len) == 0) {
                count++;
                i += len;
                matched = 1;
                break;
            }
        }
        if (matched) continue;

        // Pula espaços que não sejam \n
        if (content[i] == ' ' || content[i] == '\t') {
            i++;
            continue;
        }

        // Se for quebra de linha, conta como token
        if (content[i] == '\n') {
            count++;
            i++;
            continue;
        }

        for (int i = 0; content[i] != '\0'; i++) {
            if ((unsigned char)content[i] == 0xE2  && (unsigned char)content[i+1] == 0x80 && 
                ((unsigned char)content[i+2] == 0x9C || (unsigned char)content[i+2] == 0x9D)) {
                content[i] = '"';
            }
        }
        
        // if (strchr(0xE2, (unsigned char)content[i]) != NULL && strchr(0x80, (unsigned char)content[i+1]) != NULL && (strchr(0x9C, (unsigned char)content[i+2]) != NULL || strchr(0x9D, (unsigned char)content[i+2]) != NULL)) {
        //     content[i] = '"';
        // }

        // Se for outro caractere especial, já é um token
        if (strchr(SPECIAL_TOKENS, content[i]) != NULL) {
            count++;
            i++;
            continue;
        }

        // Senão, acumula até achar espaço ou caractere especial
        while (content[i] != '\0' &&
               content[i] != '\n' &&
               !isspace(content[i]) &&
               strchr(SPECIAL_TOKENS, content[i]) == NULL) {
            i++;
        }
        count++;
    }

    if (count == 0) {
        *length = 0;
        return NULL;
    }

    // armazenar tokens 
    char **tokens = safe_malloc(count * sizeof(char*));
    i = 0;
    int idx = 0;
    while (content[i] != '\0' && idx < count) {
        if (content[i] == ' ' || content[i] == '\t') {
            i++;
            continue;
        }

        if (content[i] == '\n') {
            tokens[idx] = safe_malloc(3); // espaço para "\\n" e '\0'
            strcpy(tokens[idx], "\\n");   // representação visual
            idx++;
            i++;
            continue;
        }

        if (strchr(SPECIAL_TOKENS, content[i]) != NULL) {
            tokens[idx] = safe_malloc(2);
            tokens[idx][0] = content[i];
            tokens[idx][1] = '\0';
            idx++;
            i++;
            continue;
        }

        int start = i;
        while (content[i] != '\0' &&
               content[i] != '\n' &&
               !isspace(content[i]) &&
               strchr(SPECIAL_TOKENS, content[i]) == NULL) {
            i++;
        }
        int len = i - start;
        tokens[idx] = safe_malloc(len + 1);
        strncpy(tokens[idx], &content[start], len);
        tokens[idx][len] = '\0';

        char *p = tokens[idx];
        while (*p){
            if ((unsigned char)p[0] == 0xE2 && (unsigned char)p[1] == 0x80 &&
                ((unsigned char)p[2] == 0x9C || (unsigned char)p[2] == 0x9D)) {
                p[0] = '"';
                memmove(p+1, p+3, strlen(p+3)+1);
            } else {
                p++;
            }
        }
        idx++;
    }

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

int check_brackets_and_quotes(char **tokens, int length) {
    char stack[length]; 
    int top = -1;
    int inside_quote = 0;

    for (int i = 0; i < length; i++) {
        char *tok = tokens[i];
        if (strcmp(tok, "\"") == 0) {
            if (!inside_quote) {
                inside_quote = 1; 
            } else {
                inside_quote = 0;
            }
            continue;
        }
        if (inside_quote) continue;
        if (strcmp(tok, "(") == 0 || strcmp(tok, "[") == 0 || strcmp(tok, "{") == 0) {
            stack[++top] = tok[0];
        }
        else if (strcmp(tok, ")") == 0 || strcmp(tok, "]") == 0 || strcmp(tok, "}") == 0) {
            if (top < 0) {
                printf("Erro: encontrou '%s' sem abertura correspondente\n", tok);
                return 0;
            }

            char open = stack[top--]; 

            if ((strcmp(tok, ")") == 0 && open != '(') ||
                (strcmp(tok, "]") == 0 && open != '[') ||
                (strcmp(tok, "}") == 0 && open != '{')) {
                printf("Erro: '%c' não combina com '%s'\n", open, tok);
                return 0; 
            }
        }
    }

    if (inside_quote) {
        printf("Erro: string aberta sem fechamento (\")\n");
        return 0;
    }

    if (top != -1) {
        char open = stack[top];
        char expected;
        if (open == '(') expected = ')';
        else if (open == '[') expected = ']';
        else expected = '}';
        printf("Faltou fechar com '%c'\n", expected);
        return 0;
    }

    return 1; 
}

int check_return_statement(char **tokens, int length) {
    bool inside_function = false;
    bool has_return = false;
    int error_count = 0;
    char *current_function_name = NULL; 

    for (int i = 0; i < length; i++) {
        char *token = tokens[i];
        if (strcmp(token, "funcao") == 0) {
            inside_function = true;
            has_return = false;
            if (i + 1 < length && strncmp(tokens[i + 1], "__", 2) == 0) {
                current_function_name = tokens[i + 1];
            } else {
                current_function_name = "funcao sem nome"; 
            }
        }
        if (inside_function && strcmp(token, "retorno") == 0) {
            has_return = true;
        }
        if (strcmp(token, "}") == 0) {
            if (inside_function) {
                if (current_function_name != NULL && strcmp(current_function_name, "__principal") != 0) {
                    if (!has_return) {
                        printf("SEMANTIC ERROR: Funcao '%s' sem 'retorno'.\n", current_function_name);
                        error_count++;
                    }
                }
                inside_function = false;
                has_return = false;
                current_function_name = NULL;
            }
        }
    }

    if (error_count > 0) {
        return 0; 
    }
    return 1; 
}

int is_invalid_operator(const char *token) {
    // Lista de caracteres válidos de operadores
    const char valid_chars[] = "<>!=&|+-*/^";
    int len = strlen(token);

    // Tokens válidos de múltiplos caracteres (válidos na gramática)
    const char* valid_multi_tokens[] = {
        "==", "<>", "<=", ">=", "&&", "||", "+", "-", "*", "/", "^", "<", ">", "="
    };
    int num_valid_multi = sizeof(valid_multi_tokens)/sizeof(valid_multi_tokens[0]);

    for (int i = 0; i < num_valid_multi; i++) {
        if (strcmp(token, valid_multi_tokens[i]) == 0) {
            return 0; 
        }
    }

    for (int i = 0; i < len; i++) {
        if (strchr(valid_chars, token[i]) == NULL) {
            return 0; 
        }
    }
    return 1;
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

        // Inicializa a tabela de símbolos para cada arquivo
        init_symbol_table();

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
            int current_line = 1;
            
            while (i < length) {
                char *cleaned = clear_token(tokens[i]);
                if (strcmp(tokens[i], "principal") == 0) {
                    PRINCIPAL_FUNC = true;
                }
                if (strcmp(tokens[i], "\\n") == 0) {
                    current_line++;
                    if (i == 0) {
                        printf("tokens[%d] = \"%s\" -> NEWLINE\n", i, tokens[i]);
                    } else {
                        char *prev = tokens[i - 1];
                        if (strcmp(prev, ";") != 0 && strcmp(prev, "{") != 0 && strcmp(prev, "}") != 0 && strcmp(prev, "\\n") != 0) {
                            printf("tokens[%d] = \"%s\" -> SYNTAX ERROR (ausência de ; após '%s')\n", i, tokens[i], prev);
                            break;
                        } else {
                            printf("tokens[%d] = \"%s\" -> NEWLINE\n", i, tokens[i]);
                        }
                    }
                    i++;
                    continue;
                } else if (strncmp(tokens[i], "\"", strlen("\"")) == 0) {
                    printf("tokens[%d] = \"%s\" -> STRING\n", i, tokens[i]);
                    i++;
                    while (i < length) {
                        size_t len = strlen(tokens[i]);
                        if (len >= strlen("\"") && strcmp(&tokens[i][len - strlen("\"")], "\"") == 0) {
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
                        
                        // Valida o nome da função
                        if (!validate_function_declaration(tokens[i + 1])) {
                            printf("ERRO ENCONTRADO: Finalizando a análise.\n");
                            break;
                        }
                        
                        printf("tokens[%d] = \"%s\" -> FUNC_NAME\n", i + 1, tokens[i + 1]);
                        
                        // Adiciona função à tabela de símbolos
                        add_symbol(tokens[i + 1], SYMBOL_FUNCTION, TYPE_VOID, current_line);
                        enter_scope(); // Entra no escopo da função
                        
                        char *current_function = tokens[i + 1];
                        
                        // Processa parâmetros da função se houver
                        i += 2;
                        if (i < length && strcmp(tokens[i], "(") == 0) {
                            int start_params = i + 1;
                            int end_params = i + 1;
                            
                            // Encontra o final da lista de parâmetros
                            while (end_params < length && strcmp(tokens[end_params], ")") != 0) {
                                end_params++;
                            }
                            
                            // Valida a lista de parâmetros se não estiver vazia
                            if (end_params > start_params) {
                                if (!validate_parameter_list(tokens, start_params, end_params - 1)) {
                                    printf("ERRO ENCONTRADO: Finalizando a análise.\n");
                                    break;
                                }
                            }
                            
                            i = start_params; // volta para processar os parâmetros
                            while (i < length && strcmp(tokens[i], ")") != 0) {
                                if (is_variable(tokens[i])) {
                                    // Remove vírgula do nome da variável se presente
                                    char *param_name = safe_malloc(strlen(tokens[i]) + 1);
                                    strcpy(param_name, tokens[i]);
                                    char *comma = strchr(param_name, ',');
                                    if (comma) *comma = '\0';
                                    
                                    // Adiciona parâmetro à tabela de símbolos
                                    add_symbol(param_name, SYMBOL_PARAMETER, TYPE_INTEGER, current_line);
                                    // Adiciona parâmetro à função
                                    add_function_parameter(current_function, param_name, TYPE_INTEGER);
                                    
                                    printf("tokens[%d] = \"%s\" -> PARAMETER\n", i, tokens[i]);
                                    free(param_name);
                                } else if (strcmp(tokens[i], ",") == 0) {
                                    printf("tokens[%d] = \"%s\" -> COMMA\n", i, tokens[i]);
                                }
                                i++;
                            }
                            if (i < length && strcmp(tokens[i], ")") == 0) {
                                i--; // volta um para o loop principal processar o ')'
                            }
                        }
                        continue;
                    } else if (i + 1 < length) {
                        printf("tokens[%d] = \"%s\" -> KEYWORD\n", i, tokens[i]);
                        printf("tokens[%d] = \"%s\" -> SEMANTIC ERROR (Nome de função deve começar com '__')\n", i + 1, tokens[i + 1]);
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
                    } else {
                        // Remove vírgula do nome da variável se presente para busca
                        char *var_name = safe_malloc(strlen(tokens[i]) + 1);
                        strcpy(var_name, tokens[i]);
                        char *comma = strchr(var_name, ',');
                        if (comma) *comma = '\0';
                        
                        // Verifica se a variável já foi declarada
                        Symbol *var = lookup_symbol(var_name);
                        if (var == NULL) {
                            printf("tokens[%d] = \"%s\" -> SEMANTIC ERROR (Variável não declarada)\n", i, tokens[i]);
                        } else {
                            printf("tokens[%d] = \"%s\" -> VARIABLE (uso)\n", i, tokens[i]);
                        }
                        free(var_name);
                    }
                } else if (strcmp(cleaned, "principal") == 0) {
                    printf("tokens[%d] = \"%s\" -> KEYWORD\n", i, tokens[i]);
                    
                    // Adiciona função principal à tabela de símbolos
                    add_symbol("__principal", SYMBOL_FUNCTION, TYPE_VOID, current_line);
                    enter_scope(); // Entra no escopo da função principal
                    
                    i++; 

                    // Verifica se o próximo token é '('
                    if (i >= length || strcmp(tokens[i], "(") != 0) {
                        printf("tokens[%d] = \"%s\" -> SYNTAX ERROR (esperado '(' após 'principal')\n", i, tokens[i]);
                        break; 
                    }
                    printf("tokens[%d] = \"%s\" -> LEFT_PAREN\n", i, tokens[i]);
                    i++; 

                    // Verifica se o próximo token é ')'
                    if (i >= length || strcmp(tokens[i], ")") != 0) {
                        printf("SYNTAX ERROR: '%s' inesperado dentro da declaração da função 'principal'\n", tokens[i]);
                        printf("tokens[%d] = \"%s\" -> SYNTAX ERROR (esperado ')' após '(' em 'principal')\n", i, tokens[i]);
                        break;
                    }
                    printf("tokens[%d] = \"%s\" -> RIGHT_PAREN\n", i, tokens[i]);
                    if (tokens[i+1] == NULL || strcmp(tokens[i+1], "{") != 0) {
                        printf("tokens[%d] = \"%s\" -> ERRO: esperado '{' após 'principal()'\n", i+1, tokens[i+1]);
                        break;
                    } 
                    i++;
                    printf("tokens[%d] = \"%s\" -> LEFT_BRACE\n", i, tokens[i]);
                    i++;
                } else if (strcmp(tokens[i], ";") == 0) {
                    printf("tokens[%d] = \"%s\" -> SEMICOLON\n", i, tokens[i]);
                } else if (strcmp(tokens[i], "+") == 0) {
                    printf("tokens[%d] = \"%s\" -> PLUS\n", i, tokens[i]);
                } else if (strcmp(tokens[i], "-") == 0) {
                    printf("tokens[%d] = \"%s\" -> MINUS\n", i, tokens[i]);
                } else if (strcmp(tokens[i], "*") == 0) {
                    printf("tokens[%d] = \"%s\" -> MULTIPLY\n", i, tokens[i]);
                } else if (strcmp(tokens[i], "/") == 0) {
                    printf("tokens[%d] = \"%s\" -> DIVIDE\n", i, tokens[i]);
                } else if (strcmp(tokens[i], "^") == 0) {
                    printf("tokens[%d] = \"%s\" -> POWER\n", i, tokens[i]);
                } else if (strcmp(tokens[i], "==") == 0) {
                    printf("tokens[%d] = \"%s\" -> EQUALS\n", i, tokens[i]);
                } else if (strcmp(tokens[i], "<>") == 0) {
                    printf("tokens[%d] = \"%s\" -> NOT_EQUALS\n", i, tokens[i]);
                } else if (strcmp(tokens[i], "<") == 0) {
                    printf("tokens[%d] = \"%s\" -> LESS\n", i, tokens[i]);
                } else if (strcmp(tokens[i], "<=") == 0) {
                    printf("tokens[%d] = \"%s\" -> LESS_EQUAL\n", i, tokens[i]);
                } else if (strcmp(tokens[i], ">") == 0) {
                    printf("tokens[%d] = \"%s\" -> GREATER\n", i, tokens[i]);
                } else if (strcmp(tokens[i], ">=") == 0) {
                    printf("tokens[%d] = \"%s\" -> GREATER_EQUAL\n", i, tokens[i]);
                } else if (strcmp(tokens[i], "&&") == 0) {
                    printf("tokens[%d] = \"%s\" -> AND\n", i, tokens[i]);
                } else if (strcmp(tokens[i], "||") == 0) {
                    printf("tokens[%d] = \"%s\" -> OR\n", i, tokens[i]);
                } else if (is_invalid_operator(tokens[i])) {
                    printf("tokens[%d] = \"%s\" -> LEXICAL ERROR (Operador inválido)\n", i, tokens[i]);
                    printf("ERRO ENCONTRADO: Finalizando a análise.\n");
                    break;
                } else if (strcmp(tokens[i], ",") == 0) {
                    printf("tokens[%d] = \"%s\" -> COMMA\n", i, tokens[i]);
                } else if (strcmp(tokens[i], "=") == 0) {
                    printf("tokens[%d] = \"%s\" -> ASSIGNMENT\n", i, tokens[i]);
                } else if (strcmp(tokens[i], ";") == 0) {
                    printf("tokens[%d] = \"%s\" -> SEMICOLON\n", i, tokens[i]);
                } else if (strcmp(tokens[i], "(") == 0) {
                    printf("tokens[%d] = \"%s\" -> LEFT_PAREN\n", i, tokens[i]);
                } else if (strcmp(tokens[i], ")") == 0) {
                    printf("tokens[%d] = \"%s\" -> RIGHT_PAREN\n", i, tokens[i]);
                } else if (strcmp(tokens[i], "{") == 0) {
                    printf("tokens[%d] = \"%s\" -> LEFT_BRACE\n", i, tokens[i]);
                    enter_scope(); // Entra em novo escopo
                } else if (strcmp(tokens[i], "}") == 0) {
                    printf("tokens[%d] = \"%s\" -> RIGHT_BRACE\n", i, tokens[i]);
                    exit_scope(); // Sai do escopo atual
                } else if (isdigit(tokens[i][0])) {
                    printf("tokens[%d] = \"%s\" -> INTEGER\n", i, tokens[i]);
                } else if (strcmp(tokens[i], "leia") == 0) {
                    // Processamento específico para o comando leia
                    printf("tokens[%d] = \"%s\" -> LEIA_COMMAND\n", i, tokens[i]);
                    
                    if (i + 1 < length) {
                        int end_pos = length - 1;
                        if (validate_leia_command(tokens, i + 1, &end_pos, current_line)) {
                            // Processa tokens validados do comando leia
                            for (int j = i + 1; j <= end_pos; j++) {
                                if (strcmp(tokens[j], "(") == 0) {
                                    printf("tokens[%d] = \"%s\" -> LEFT_PAREN\n", j, tokens[j]);
                                } else if (strcmp(tokens[j], ")") == 0) {
                                    printf("tokens[%d] = \"%s\" -> RIGHT_PAREN\n", j, tokens[j]);
                                } else if (strcmp(tokens[j], ",") == 0) {
                                    printf("tokens[%d] = \"%s\" -> COMMA\n", j, tokens[j]);
                                } else if (strcmp(tokens[j], ";") == 0) {
                                    printf("tokens[%d] = \"%s\" -> SEMICOLON\n", j, tokens[j]);
                                } else if (is_variable(tokens[j])) {
                                    printf("tokens[%d] = \"%s\" -> VARIABLE (leitura)\n", j, tokens[j]);
                                }
                            }
                            i = end_pos; // Pula para o final do comando processado
                        } else {
                            printf("ERRO ENCONTRADO: Finalizando a análise.\n");
                            break;
                        }
                    } else {
                        printf("SYNTAX ERROR: Comando 'leia' incompleto\n");
                        break;
                    }
                } else if (strcmp(tokens[i], "escreva") == 0 || strcmp(tokens[i], "se") == 0 || strcmp(tokens[i], "para") == 0) {
                    // Verifica se a próxima string começa com '('
                    if (tokens[i+1] != NULL && strncmp(tokens[i+1], "(", 1) == 0) {
                        printf("tokens[%d] = \"%s\" -> KEYWORD_FUNC\n", i, tokens[i]);
                        printf("tokens[%d] = \"%s\" -> KEYWORD_FUNC_LEFT_PAREN\n", i, tokens[i+1]);
                        i++;
                    } else {
                        printf("tokens[%d] = \"%s\" -> FUNC\n", i, tokens[i]);
                        printf("tokens[%d] = \"%s\" -> LEXICAL ERROR - Era esperado '(' após nome de função\n", i + 1, tokens[i+1]);
                        break;
                    }
                } else if (strcmp(tokens[i], "inteiro") == 0 || strcmp(tokens[i], "texto") == 0 || strcmp(tokens[i], "decimal") == 0) {
                    // Declaração de variável
                    DataType var_type = string_to_data_type(tokens[i]);
                    const char* type_name = data_type_to_string(var_type);
                    
                    printf("tokens[%d] = \"%s\" -> %s_TYPE\n", i, tokens[i], 
                           var_type == TYPE_INTEGER ? "INTEGER" : 
                           var_type == TYPE_STRING ? "STRING" : "FLOAT");
                    
                    // Processa todas as variáveis declaradas na linha
                    i++; // vai para o primeiro identificador
                    while (i < length && strcmp(tokens[i], ";") != 0 && strcmp(tokens[i], "\\n") != 0) {
                        if (strncmp(tokens[i], "!", 1) == 0) {
                            // Remove vírgula do nome da variável se presente
                            char *var_name = safe_malloc(strlen(tokens[i]) + 1);
                            strcpy(var_name, tokens[i]);
                            char *comma = strchr(var_name, ',');
                            if (comma) *comma = '\0';
                            
                            // Verifica se é redeclaração de parâmetro
                            if (is_parameter_redeclaration(var_name)) {
                                printf("tokens[%d] = \"%s\" -> SEMANTIC ERROR (Parâmetro '%s' não deve ser redeclarado dentro da função)\n", 
                                       i, tokens[i], var_name);
                                printf("ERRO ENCONTRADO: Finalizando a análise.\n");
                                free(var_name);
                                break;
                            }
                            
                            // Adiciona variável à tabela de símbolos
                            if (add_symbol(var_name, SYMBOL_VARIABLE, var_type, current_line)) {
                                printf("tokens[%d] = \"%s\" -> VARIABLE (declaração)\n", i, tokens[i]);
                            }
                            free(var_name);
                        } else if (strcmp(tokens[i], "=") == 0) {
                            printf("tokens[%d] = \"%s\" -> ASSIGNMENT\n", i, tokens[i]);
                        } else if (strcmp(tokens[i], ",") == 0) {
                            printf("tokens[%d] = \"%s\" -> COMMA\n", i, tokens[i]);
                        } else if (isdigit(tokens[i][0])) {
                            printf("tokens[%d] = \"%s\" -> INTEGER\n", i, tokens[i]);
                        } else {
                            printf("tokens[%d] = \"%s\" -> IDENTIFIER/OTHER\n", i, tokens[i]);
                        }
                        i++;
                    }
                    i--; // volta um para o loop principal processar o próximo token
                } else if (is_variable(tokens[i])) {
                    // Remove vírgula do nome da variável se presente para busca
                    char *var_name = safe_malloc(strlen(tokens[i]) + 1);
                    strcpy(var_name, tokens[i]);
                    char *comma = strchr(var_name, ',');
                    if (comma) *comma = '\0';
                    
                    // Verifica se a variável já foi declarada
                    Symbol *var = lookup_symbol(var_name);
                    if (var == NULL) {
                        printf("tokens[%d] = \"%s\" -> SEMANTIC ERROR (Variável não declarada)\n", i, tokens[i]);
                    } else {
                        printf("tokens[%d] = \"%s\" -> VARIABLE (uso)\n", i, tokens[i]);
                    }
                    free(var_name);
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
                } else if (strncmp(tokens[i], "__", 2) == 0) {
                    // Possível chamada de função
                    if (i + 1 < length && strcmp(tokens[i + 1], "(") == 0) {
                        // Conta parâmetros na chamada
                        int param_count = 0;
                        int j = i + 2; // Pula o '('
                        int paren_level = 1;
                        
                        while (j < length && paren_level > 0) {
                            if (strcmp(tokens[j], "(") == 0) {
                                paren_level++;
                            } else if (strcmp(tokens[j], ")") == 0) {
                                paren_level--;
                            } else if (strcmp(tokens[j], ",") == 0 && paren_level == 1) {
                                param_count++;
                            } else if (paren_level == 1 && strcmp(tokens[j], ",") != 0 && 
                                     strcmp(tokens[j], "(") != 0 && strcmp(tokens[j], ")") != 0) {
                                // Se há pelo menos um parâmetro não-vazio
                                if (param_count == 0) param_count = 1;
                            }
                            j++;
                        }
                        
                        // Se havia vírgulas, incrementa para contar o último parâmetro
                        if (param_count > 0 && j > i + 3) param_count++;
                        
                        if (validate_function_call(tokens[i], param_count, current_line)) {
                            printf("tokens[%d] = \"%s\" -> FUNCTION_CALL (%d parâmetros)\n", i, tokens[i], param_count);
                        } else {
                            printf("tokens[%d] = \"%s\" -> SEMANTIC ERROR (Chamada de função inválida)\n", i, tokens[i]);
                        }
                    } else {
                        printf("tokens[%d] = \"%s\" -> IDENTIFIER/OTHER\n", i, tokens[i]);
                    }
                } else {
                    printf("tokens[%d] = \"%s\" -> IDENTIFIER/OTHER\n", i, tokens[i]);
                }
                i++;
            }

            check_brackets_and_quotes(tokens, length);

            if (!check_return_statement(tokens, length)) {
                printf("Verificação de 'retorno' falhou. Erro encontrado.\n");
            } else {
                printf("Verificação de 'retorno' concluída com sucesso.\n");
            }

            // Infere tipos de parâmetros antes de imprimir a tabela
            infer_parameter_types();

            // Imprime a tabela de símbolos
            print_symbol_table();

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
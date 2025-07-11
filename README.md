# Analisador Léxico - Compiladores

Primeira parte do trabalho de Compiladores do curso de Ciência da Computação do IFG-Anápolis.

## 📝 Descrição

Este projeto implementa um **analisador léxico** para identificar e classificar tokens de uma linguagem de programação simplificada, com foco especial na **detecção de erros léxicos**. O analisador processa arquivos de código fonte e categoriza cada token encontrado, identificando quando há palavras-chave escritas incorretamente ou outras inconsistências lexicais.

## 🎯 Objetivos

- **Identificar erros léxicos** em código fonte
- **Classificar tokens** em categorias (palavras-chave, variáveis, identificadores, etc.)
- **Detectar similaridades** entre tokens incorretos e palavras-chave válidas
- **Processar múltiplos arquivos** automaticamente

## 🔧 Funcionalidades

### Classificação de Tokens
O analisador identifica os seguintes tipos de tokens:

- **KEYWORD**: Palavras-chave da linguagem (`principal`, `inteiro`, `escreva`, `leia`, etc.)
- **VARIABLE**: Variáveis (devem começar com `!`)
- **FUNC_NAME**: Nomes de funções (devem começar com `__`)
- **INTEGER**: Números inteiros
- **IDENTIFIER/OTHER**: Outros identificadores válidos
- **LEFT_PAREN/RIGHT_PAREN**: Parênteses `(` e `)`
- **LEFT_BRACE/RIGHT_BRACE**: Chaves `{` e `}`
- **SEMICOLON**: Ponto e vírgula `;`
- **LEXICAL ERROR**: Tokens com erros léxicos

### Detecção de Erros Léxicos
O sistema utiliza o **algoritmo de distância de Levenshtein** para detectar:

- Palavras-chave escritas incorretamente (ex: `pincipal` → `principal`)
- Variações com caracteres faltando ou extras
- Prefixos de palavras-chave válidas

### Linguagem Suportada

#### Palavras-chave:
- `principal` - função principal
- `inteiro` - tipo de dados inteiro
- `retorno` - comando de retorno
- `escreva` - comando de saída
- `leia` - comando de entrada
- `funcao` - declaração de função
- `se` - condicional if
- `senao` - condicional else
- `para` - laço for

#### Regras de Sintaxe:
- **Variáveis**: devem começar com `!` (ex: `!a`, `!numero`)
- **Funções**: devem começar com `__` (ex: `__soma`)
- **Strings**: delimitadas por aspas duplas

## 📁 Estrutura do Projeto

```
compiler/
├── main.c                 # Código principal do analisador
├── main                   # Executável compilado
├── ASCII.TXT             # Arquivo de referência ASCII
├── data/                 # Arquivos de teste (.txt)
│   ├── exemplo_1.txt
│   ├── exemplo_2.txt
│   ├── exemplo_3.txt
│   ├── exemplo_4.txt
│   ├── Erro_exemplo_5.txt
│   ├── Erro_exemplo_6.txt
│   ├── Erro_Exemplo_7.txt
│   ├── Erro_Exemplo_8.txt
│   └── Erro_Exemplo_9.txt
└── datartf/              # Arquivos de teste (.rtf)
    └── [mesmos arquivos em formato RTF]
```

## 🚀 Como Executar

### Pré-requisitos
- Compilador GCC
- Sistema Linux/Unix

### Compilação
```bash
gcc -o main main.c
```

### Execução
```bash
./main
```

O programa irá processar automaticamente todos os arquivos na pasta `data/` e exibir:
- Lista de tokens encontrados
- Classificação de cada token
- Detecção de erros léxicos
- Uso de memória

## 📊 Exemplo de Saída

```
==============================
Processando arquivo: ./data/Erro_Exemplo_9.txt

Total de tokens: 13

Classificação dos tokens:
tokens[0] = "principal" -> KEYWORD
tokens[1] = "(" -> LEFT_PAREN
tokens[2] = ")" -> RIGHT_PAREN
tokens[3] = "{" -> LEFT_BRACE
tokens[4] = "inteiro" -> KEYWORD
tokens[5] = "!a" -> VARIABLE
tokens[6] = "!b2" -> VARIABLE
tokens[7] = "=" -> IDENTIFIER/OTHER
tokens[8] = "7" -> INTEGER
tokens[9] = ";" -> SEMICOLON
tokens[10] = "escrva" -> LEXICAL ERROR

Memória ocupada: 2048 Bytes ou 2.00 KB
```

## 🔍 Recursos Técnicos

### Gerenciamento de Memória
- **Limite de memória**: 2MB (2048 KB)
- **Alocação segura** com verificação de limites
- **Liberação automática** de memória

### Processamento de Arquivo
- **Leitura completa** do arquivo em memória
- **Remoção de BOM UTF-8** quando presente
- **Tokenização** baseada em delimitadores

### Algoritmos Implementados
- **Distância de Levenshtein** para detecção de similaridade
- **Tokenização** com múltiplos delimitadores
- **Classificação automática** de tokens

## 👥 Contribuição

**Desenvolvido por:**
- [Davi Galdino](https://github.com/davi-ga)
- [Ana Misque](https://github.com/mxtqnt)

## 📄 Licença

Este projeto foi desenvolvido para fins acadêmicos como parte do curso de Compiladores.

---

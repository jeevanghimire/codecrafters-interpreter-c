#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_TOKEN_OUTPUT 4096

const char *reserved[] = {
    "and", "class", "else", "false", "for", "fun", "if", "nil",
    "or", "print", "return", "super", "this", "true", "var", "while"
};
const char *reservedU[] = {
    "AND", "CLASS", "ELSE", "FALSE", "FOR", "FUN", "IF", "NIL",
    "OR", "PRINT", "RETURN", "SUPER", "THIS", "TRUE", "VAR", "WHILE"
};

int is_equal(const char *a, const char *b) {
    return strcmp(a, b) == 0;
}

char *read_file(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Error: can't open file %s\n", filename);
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    char *buffer = malloc(size + 1);
    fread(buffer, 1, size, f);
    buffer[size] = '\0';
    fclose(f);
    return buffer;
}
void printString(){
    
}

int tokenize(const char *src, char *output) {
    size_t len = strlen(src);
    size_t line = 1;
    int error = 0;
    char *out = output;

    for (size_t i = 0; i < len; i++) {
        char c = src[i];

        switch (c) {
            case '(': out += sprintf(out, "LEFT_PAREN ( null\n"); break;
            case ')': out += sprintf(out, "RIGHT_PAREN ) null\n"); break;
            case '{': out += sprintf(out, "LEFT_BRACE { null\n"); break;
            case '}': out += sprintf(out, "RIGHT_BRACE } null\n"); break;
            case '*': out += sprintf(out, "STAR * null\n"); break;
            case '+': out += sprintf(out, "PLUS + null\n"); break;
            case '-': out += sprintf(out, "MINUS - null\n"); break;
            case ';': out += sprintf(out, "SEMICOLON ; null\n"); break;
            case ',': out += sprintf(out, "COMMA , null\n"); break;
            case '.': out += sprintf(out, "DOT . null\n"); break;

            case '=':
                if (src[i+1] == '=') { out += sprintf(out, "EQUAL_EQUAL == null\n"); i++; }
                else { out += sprintf(out, "EQUAL = null\n"); }
                break;

            case '!':
                if (src[i+1] == '=') { out += sprintf(out, "BANG_EQUAL != null\n"); i++; }
                else { out += sprintf(out, "BANG ! null\n"); }
                break;

            case '<':
                if (src[i+1] == '=') { out += sprintf(out, "LESS_EQUAL <= null\n"); i++; }
                else { out += sprintf(out, "LESS < null\n"); }
                break;

            case '>':
                if (src[i+1] == '=') { out += sprintf(out, "GREATER_EQUAL >= null\n"); i++; }
                else { out += sprintf(out, "GREATER > null\n"); }
                break;

            case '/':
                if (src[i+1] == '/') {
                    while (i < len && src[i] != '\n') i++;  // skip comment
                    line++;
                } else {
                    out += sprintf(out, "SLASH / null\n");
                }
                break;

            case '"': {
                char str[256];
                int j = 0;
                i++;
                while (i < len && src[i] != '"' && src[i] != '\n') {
                    str[j++] = src[i++];
                }
                str[j] = '\0';
                if (src[i] != '"') {
                    fprintf(stderr, "[line %lu] Error: Unterminated string\n", line);
                    error = 1;
                } else {
                    out += sprintf(out, "STRING \"%s\" %s\n", str, str);
                }
                break;
            }

            case '\n': line++; break;
            case ' ':
            case '\t':
                break; // ignore whitespace

            default:
                if (isdigit(c)) {
                    char num[64];
                    int j = 0;
                    while (i < len && (isdigit(src[i]) || src[i] == '.')) {
                        num[j++] = src[i++];
                    }
                    i--;
                    num[j] = '\0';
                    out += sprintf(out, "NUMBER %s %s\n", num, num);
                }
                else if (isalpha(c) || c == '_') {
                    char id[64];
                    int j = 0;
                    while (i < len && (isalnum(src[i]) || src[i] == '_')) {
                        id[j++] = src[i++];
                    }
                    i--;
                    id[j] = '\0';

                    int is_keyword = 0;
                    for (int k = 0; k < 16; ++k) {
                        if (is_equal(reserved[k], id)) {
                            out += sprintf(out, "%s %s null\n", reservedU[k], reserved[k]);
                            is_keyword = 1;
                            break;
                        }
                    }
                    if (!is_keyword) {
                        out += sprintf(out, "IDENTIFIER %s null\n", id);
                    }
                }
                else {
                    fprintf(stderr, "[line %lu] Error: Unknown character '%c'\n", line, c);
                    error = 1;
                }
                break;
        }
    }

    out += sprintf(out, "EOF null\n");
    return error;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <tokenize|parse> <filename>\n", argv[0]);
        return 1;
    }

    char *command = argv[1];
    char *filename = argv[2];

    char *source = read_file(filename);
    if (!source) return 1;

    char tokens[MAX_TOKEN_OUTPUT];
    int err = tokenize(source, tokens);

    if (is_equal(command, "tokenize")) {
        printf("%s", tokens);
    }
    else if (is_equal(command, "parse")) {
        char *line = strtok(tokens, "\n");
        while (line) {
            char *type = strtok(line, " ");
            strtok(NULL, " "); // skip raw token
            char *value = strtok(NULL, " ");

            if (is_equal(type, "TRUE") || is_equal(type, "FALSE") || is_equal(type, "NIL") || is_equal(type, "NUMBER")) {
                printf("%s\n", value);
            }

            line = strtok(NULL, "\n");
        }
    } 
    // print String for String Literal 
    else if (is_equal(command, "print")) {
        char *line = strtok(tokens, "\n");
        while (line) {
            char *type = strtok(line, " ");
            strtok(NULL, " "); // skip raw token
            char *value = strtok(NULL, " ");

            if (is_equal(type, "STRING")) {
                printf("%s\n", value);
            }

            line = strtok(NULL, "\n");
        }
    }
    // print Identifier for Identifier Literal 
    else if (is_equal(command, "identifier")) {
        char *line = strtok(tokens, "\n");
        while (line) {
            char *type = strtok(line, " ");
            strtok(NULL, " "); // skip raw token
            char *value = strtok(NULL, " ");

            if (is_equal(type, "IDENTIFIER")) {
                printf("%s\n", value);
            }

            line = strtok(NULL, "\n");
        }
    }
    // print Number for Number Literal 
    else if (is_equal(command, "number")) {
        char *line = strtok(tokens, "\n");
        while (line) {
            char *type = strtok(line, " ");
            strtok(NULL, " "); // skip raw token
            char *value = strtok(NULL, " ");

            if (is_equal(type, "NUMBER")) {
                printf("%s\n", value);
            }

            line = strtok(NULL, "\n");
        }
    }
    // print Boolean for Boolean Literal 
    else if (is_equal(command, "boolean")) {
        char *line = strtok(tokens, "\n");
        while (line) {
            char *type = strtok(line, " ");
            strtok(NULL, " "); // skip raw token
            char *value = strtok(NULL, " ");

            if (is_equal(type, "TRUE") || is_equal(type, "FALSE")) {
                printf("%s\n", value);
            }

            line = strtok(NULL, "\n");
        }
    }
    else {
        fprintf(stderr, "Unknown command: %s\n", command);
    }

    free(source);
    return err;
}

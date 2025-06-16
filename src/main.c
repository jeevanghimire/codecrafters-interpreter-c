#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *read_file_contents(const char *filename);

int main(int argc, char *argv[])
{
    // Disable output buffering
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
    char exit_code = 0;

    if (argc < 3)
    {
        fprintf(stderr, "Usage: ./your_program tokenize <filename>\n");
        return 1;
    }

    const char *command = argv[1];

    if (strcmp(command, "tokenize") == 0)
    {
        // You can use print statements as follows for debugging, they'll be visible
        // when running tests.
        fprintf(stderr, "Logs from your program will appear here!\n");

        char *file_contents = read_file_contents(argv[2]);

        size_t line = 1;
        for (long cursor = 0; file_contents[cursor] != '\0'; ++cursor)
        {
            char c = file_contents[cursor];
            switch (c)
            {
            case '(':
                printf("LEFT_PAREN ( null\n");
                break;
            case ')':
                printf("RIGHT_PAREN ) null\n");
                break;
            case '{':
                printf("LEFT_BRACE { null\n");
                break;
            case '}':
                printf("RIGHT_BRACE } null\n");
                break;
            case ',':
                printf("COMMA , null\n");
                break;
            case '.':
                printf("DOT . null\n");
                break;
            case '-':
                printf("MINUS - null\n");
                break;
            case '+':
                printf("PLUS + null\n");
                break;
            case ';':
                printf("SEMICOLON ; null\n");
                break;
            case '/':
                if (file_contents[cursor + 1] == '/')
                {
                    while (file_contents[cursor] != '\n' && file_contents[cursor] != '\0')
                        cursor++;
                    line++;
                }
                else if (file_contents[cursor + 1] == '*')
                {
                    cursor += 2;
                    while (!(file_contents[cursor] == '*' && file_contents[cursor + 1] == '/') &&
                           file_contents[cursor] != '\0')
                    {
                        if (file_contents[cursor] == '\n')
                            line++;
                        cursor++;
                    }
                    if (file_contents[cursor] == '*')
                        cursor++;

                    // If the next character is the closing slash, then consume it.
                    if (file_contents[cursor] == '/')
                        cursor++;
                }
                else
                {
                    printf("SLASH / null\n");
                }
                break;
            case ' ':
            case '\r':
            case '\t':
                break;
            case '\n':
                line++;
                break;
            case '*':
                printf("STAR * null\n");
                break;
            case '=':
                if (file_contents[cursor + 1] == '=')
                {
                    printf("EQUAL_EQUAL == null\n");
                    cursor++;
                }
                else
                {
                    printf("EQUAL = null\n");
                }
                break;
            case '!':
                if (file_contents[cursor + 1] == '=')
                {
                    printf("BANG_EQUAL != null\n");
                    cursor++;
                }
                else
                {
                    printf("BANG ! null\n");
                }
                break;
            case '<':
                if (file_contents[cursor + 1] == '=')
                {
                    printf("LESS_EQUAL <= null\n");
                    cursor++;
                }
                else
                {
                    printf("LESS < null\n");
                }
                break;
            case '>':
                if (file_contents[cursor + 1] == '=')
                {
                    printf("GREATER_EQUAL >= null\n");
                    cursor++;
                }
                else
                {
                    printf("GREATER > null\n");
                }
                break;
                case '"':
                {
                    size_t start = ++cursor;
                    while (file_contents[cursor] != '"' && file_contents[cursor] != '\n' && file_contents[cursor] != '\0')
                        cursor++;
                    if (file_contents[cursor] == '"')
                    {
                        size_t length = cursor - start;
                        char *string_literal = malloc(length + 1);
                        strncpy(string_literal, &file_contents[start], length);
                        string_literal[length] = '\0';
                        printf("STRING \"%s\" %s\n", string_literal, string_literal);
                        free(string_literal);
                    }
                    else
                    {
                        fprintf(stderr, "[line %zu] Error: Unterminated string.\n", line);
                        exit_code = 65; // Exit code for error
                    }
                    break;
                }
                default:
                if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')
                {
                    size_t start = cursor;
                    while ((file_contents[cursor] >= 'a' && file_contents[cursor] <= 'z') ||
                            (file_contents[cursor] >= 'A' && file_contents[cursor] <= 'Z') ||
                            (file_contents[cursor] >= '0' && file_contents[cursor] <= '9') ||
                            file_contents[cursor] == '_')
                    {
                        cursor++;
                    }
                    size_t length = cursor - start;
                    char *identifier = malloc(length + 1);
                    strncpy(identifier, &file_contents[start], length);
                    identifier[length] = '\0';
                    char CapitalizedIdentifier[256];
                    for (size_t i = 0; i < length; i++)
                    {
                        CapitalizedIdentifier[i] = identifier[i];
                        if (CapitalizedIdentifier[i] >= 'a' && CapitalizedIdentifier[i] <= 'z')
                        {
                            CapitalizedIdentifier[i] -= 32; // Convert to uppercase
                        }
                    }
                    CapitalizedIdentifier[length] = '\0';

                    // List of keywords to check.
                    const char *keywords[] = {
                        "and", "class", "else", "false",
                        "for", "fun", "if", "nil",
                        "or", "print", "return", "super",
                        "this", "true", "var", "while"
                    };
                    int isKeyword = 0;
                    for (size_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++)
                    {
                        if (strcmp(identifier, keywords[i]) == 0)
                        {
                            isKeyword = 1;
                            break;
                        }
                    }
                    if (isKeyword)
                    {
                        printf("IDENTIFIER Keyword null\n");
                    }
                    else
                    {
                        printf("%s %s null\n", CapitalizedIdentifier, identifier);
                    }
                    free(identifier);
                    free(identifier);
                    // Decrement to counter the for-loop's auto increment
                    cursor--;
                }
                else if (c >= '0' && c <= '9')
                {
                    // Handle numbers: both integer and floating-point.
                    size_t start = cursor;
                    while (file_contents[cursor] >= '0' && file_contents[cursor] <= '9')
                        cursor++;
                    if (file_contents[cursor] == '.')
                    {
                        cursor++;
                        while (file_contents[cursor] >= '0' && file_contents[cursor] <= '9')
                            cursor++;

                        size_t length = cursor - start;
                        char *raw_number = malloc(length + 1);
                        strncpy(raw_number, &file_contents[start], length);
                        raw_number[length] = '\0';

                        char *parsed_number = strdup(raw_number);
                        char *dot = strchr(parsed_number, '.');
                        if (dot)
                        {
                            char *end = parsed_number + strlen(parsed_number) - 1;
                            while (end > dot && *end == '0')
                            {
                                *end = '\0';
                                end--;
                            }
                            if (end == dot)
                            {
                                end++;
                                *end++ = '0';
                                *end = '\0';
                            }
                        }
                        printf("NUMBER %s %s\n", raw_number, parsed_number);
                        free(raw_number);
                        free(parsed_number);
                    }
                    else
                    {
                        size_t length = cursor - start;
                        char *number = malloc(length + 1);
                        strncpy(number, &file_contents[start], length);
                        number[length] = '\0';
                        printf("NUMBER %s %s.0\n", number, number);
                        free(number);
                    }
                    // Decrement to counter the for-loop's auto increment and avoid skipping the next character.
                    cursor--;
                }
                else
                {
                    // Handle unexpected characters.
                    fprintf(stderr, "[line %zu] Error: Unexpected character: %c\n", line, c);
                    exit_code = 65; // Exit code for error
                }
                break;
            }
        }
        printf("EOF  null\n"); // Placeholder, replace this line when implementing
                               // the scanner

        free(file_contents);
    }
    else
    {
        fprintf(stderr, "Unknown command: %s\n", command);
        return 1;
    }

    return exit_code;
}

char *read_file_contents(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Error reading file: %s\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    char *file_contents = malloc(file_size + 1);
    if (file_contents == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(file_contents, 1, file_size, file);
    if (bytes_read < file_size)
    {
        fprintf(stderr, "Error reading file contents\n");
        free(file_contents);
        fclose(file);
        return NULL;
    }

    file_contents[file_size] = '\0';
    fclose(file);

    return file_contents;
}
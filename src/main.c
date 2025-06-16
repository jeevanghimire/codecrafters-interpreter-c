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
            if (c == '(')
                printf("LEFT_PAREN ( null\n");
            else if (c == ')')
                printf("RIGHT_PAREN ) null\n");
            else if (c == '{')
                printf("LEFT_BRACE { null\n");
            else if (c == '}')
                printf("RIGHT_BRACE } null\n");
            else if (c == ',')
                printf("COMMA , null\n");
            else if (c == '.')
                printf("DOT . null\n");
            else if (c == '-')
                printf("MINUS - null\n");
            else if (c == '+')
                printf("PLUS + null\n");
            else if (c == ';')
                printf("SEMICOLON ; null\n");
            else if (c == '/')
            {
                if (file_contents[cursor + 1] == '/')
                {
                    // Skip single-line comment
                    while (file_contents[cursor] != '\n' && file_contents[cursor] != '\0')
                        cursor++;
                }
                else if (file_contents[cursor + 1] == '*')
                {
                    // Skip multi-line comment
                    cursor += 2; // Skip the '/*'
                    while (!(file_contents[cursor] == '*' && file_contents[cursor + 1] == '/') &&
                            file_contents[cursor] != '\0')
                    {
                        if (file_contents[cursor] == '\n')
                            line++;
                        cursor++;
                    }
                    if (file_contents[cursor] == '*')
                        cursor++; // Skip the '*'
                    if (file_contents[cursor] == '/')
                        cursor++; // Skip the '/'
                }
                else
                {
                    printf("SLASH / null\n");
                }
            }
            else if (c == ' ')
            {
                // Ignore whitespace
            }
            else if (c == '\n')
            {
                line++;
            }

            else if (c == '*')
                printf("STAR * null\n");
            else if (c == '=' && file_contents[cursor + 1] == '=')
            {
                printf("EQUAL_EQUAL == null\n");
                cursor++; // Skip the next character
            }
            else if (c == '=')
                printf("EQUAL = null\n");
            else if (c == '!')
            {
                if (file_contents[cursor + 1] == '=')
                {
                    printf("BANG_EQUAL != null\n");
                    cursor++; // Skip the next character
                }
                else
                {
                    printf("BANG ! null\n");
                }
            }
            else if (c == '<')
            {
                if (file_contents[cursor + 1] == '=')
                {
                    printf("LESS_EQUAL <= null\n");
                    cursor++; // Skip the next character
                }
                else
                {
                    printf("LESS < null\n");
                }
            }
            else if (c == '>')
            {
                if (file_contents[cursor + 1] == '=')
                {
                    printf("GREATER_EQUAL >= null\n");
                    cursor++; // Skip the next character
                }
                else
                {
                    printf("GREATER > null\n");
                }
            }
            else if (c == '"')
            {
                size_t start = ++cursor;
                while (file_contents[cursor] != '"' && file_contents[cursor] != '\0')
                    cursor++;

                if (file_contents[cursor] == '"')
                {
                    size_t length = cursor - start;
                    char *string_literal = malloc(length + 1);
                    strncpy(string_literal, &file_contents[start], length);
                    string_literal[length] = '\0';
                    printf("STRING \"%s\" null\n", string_literal);
                    free(string_literal);
                }
                else
                {
                    fprintf(stderr, "[line %zu] Error: Unterminated string.\n", line);
                    exit_code = 65;
                }
            }
            else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')
            {
                size_t start = cursor;
                while ((file_contents[cursor] >= 'a' && file_contents[cursor] <= 'z') ||
                        (file_contents[cursor] >= 'A' && file_contents[cursor] <= 'Z') ||
                        (file_contents[cursor] >= '0' && file_contents[cursor] <= '9') ||
                        file_contents[cursor] == '_')
                    cursor++;

                size_t length = cursor - start;
                char *identifier =
                    malloc(length + 1);
                strncpy(identifier, &file_contents[start], length);
                identifier[length] = '\0';
            }
            
            else
            {
                fprintf(stderr, "[line %zu] Error: Unexpected character: %c\n", line,
                        c);
                exit_code = 65;
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
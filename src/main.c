#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define SUBSTR_BUFFER_SZ 1024

char *read_file_contents(const char *filename);
int scan_file(const char *filecontent);

int main(int argc, char *argv[])
{
    // Disable output buffering
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    if (argc < 3)
    {
        fprintf(stderr, "Usage: ./your_program tokenize <filename>\n");
        return 1;
    }

    const char *command = argv[1];

    if (strcmp(command, "tokenize") == 0)
    {
        char *file_contents = read_file_contents(argv[2]);

        int scan_result;
        if ((scan_result = scan_file(file_contents)), scan_result > 0)
        {
            exit(scan_result);
        }

        free(file_contents);
    }
    else
    {
        fprintf(stderr, "Unknown command: %s\n", command);
        return 1;
    }

    return 0;
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

bool next_is(char c, const char **current)
{
    char current_c = **current;
    char next = *(*current + 1);
    if (current_c == '\0' || next == '\0')
        return false;
    if (next != c)
        return false;

    ++(*current);

    return true;
}

const char *advance_before(const char *start, const char *matches)
{
    size_t matches_len = strlen(matches);

    size_t i = 1;
    bool found_match = false;
    do
    {
        for (size_t m = 0; m < matches_len; ++m)
        {
            if (start[i] == matches[m])
            {
                found_match = true;
                break;
            }
        }

        // handle null term as special case, strlen wont take it into account
        if (start[i] == '\0')
            found_match = true;

        if (!found_match)
            ++i;
        else
            break;

    } while (true);

    return start + i - 1;
}

// Advances start while the provided test function returns true or null
// terminator is reached. Returns the character pointer where the test failed or
// null terminator.
const char *advance_while(const char *start, bool (*test)(char))
{
    // might not be too memory safe lol
    if (*start == '\0')
        return start;

    size_t i = 1;
    for (; start[i] != '\0'; ++i)
    {
        if (!test(start[i]))
            break;
    }

    return start + i;
}

// filters for advancing cursor //

bool not_newline(char c) { return c != '\n'; }

bool not_doublequote_nor_newline(char c) { return c != '\n' && c != '"'; }

bool alpha_or_underscore_or_digit(char c)
{
    return isalpha(c) || isdigit(c) || c == '_';
}

// -- //

#define NUM_RESERVED_KEYWORDS 16
// storing uppercase avoids a malloc when printing, we are comparing with
// lowercase characters
static char *reserved_keywords[16] = {
    "AND",
    "CLASS",
    "ELSE",
    "FALSE",
    "FOR",
    "FUN",
    "IF",
    "NIL",
    "OR",
    "PRINT",
    "RETURN",
    "SUPER",
    "THIS",
    "TRUE",
    "VAR",
    "WHILE",
};
// a bit overkill optim
static size_t reserved_keyword_lens[NUM_RESERVED_KEYWORDS];

bool print_keyword(const char *start, size_t len)
{
    bool ret = false;

    for (size_t k = 0; k < NUM_RESERVED_KEYWORDS; ++k)
    {
        if (len != reserved_keyword_lens[k])
            continue;

        bool found_match = true;
        for (size_t i = 0; i < len; ++i)
        {
            if (start[i] != tolower(reserved_keywords[k][i]))
            {
                found_match = false;
                break;
            }
        }

        if (found_match)
        {
            ret = true;
            printf("%s %.*s null\n", reserved_keywords[k], (int)len, start);
        }
    }

    return ret;
}

int scan_file(const char *filecontent)
{

    // preparation steps //
    for (size_t i = 0; i < NUM_RESERVED_KEYWORDS; ++i)
        reserved_keyword_lens[i] = strlen(reserved_keywords[i]);
    // -- //

    int retcode = 0;
    int line = 1;
    const char *current = filecontent;

    while (*current)
    {
        switch (*current)
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
        case '.':
            printf("DOT . null\n");
            break;
        case ',':
            printf("COMMA , null\n");
            break;
        case '*':
            printf("STAR * null\n");
            break;
        case '+':
            printf("PLUS + null\n");
            break;
        case '-':
            printf("MINUS - null\n");
            break;
        case ';':
            printf("SEMICOLON ; null\n");
            break;
        case '/':
            if (next_is('/', &current))
            {
                current = advance_while(current, not_newline);
                // let the next iteration handle the character
                current -= 1;
            }
            else
            {
                printf("SLASH / null\n");
            }
            break;
        case '=':
            if (next_is('=', &current))
            {
                printf("EQUAL_EQUAL == null\n");
            }
            else
            {
                printf("EQUAL = null\n");
            }
            break;
        case '!':
            if (next_is('=', &current))
            {
                printf("BANG_EQUAL != null\n");
            }
            else
            {
                printf("BANG ! null\n");
            }
            break;
        case '>':
            if (next_is('=', &current))
            {
                printf("GREATER_EQUAL >= null\n");
            }
            else
            {
                printf("GREATER > null\n");
            }
            break;
        case '<':
            if (next_is('=', &current))
            {
                printf("LESS_EQUAL <= null\n");
            }
            else
            {
                printf("LESS < null\n");
            }
            break;

        case '\n':
            ++line;
            break;
        case ' ':
        case '\t':
            break;

        case '"':
        {
            const char *start = current;
            const char *cursor = advance_while(start, not_doublequote_nor_newline);
            if (*cursor != '"')
            {
                fprintf(stderr, "[line %d] Error: Unterminated string.\n", line);
                retcode = 65;

                // go before cursor so the next iteration handles whatever character
                // that is
                current = cursor - 1;
            }
            else
            {
                ++start; // move past double quote
                int num_chars = (int)(cursor - start);
                printf("STRING \"%.*s\" %.*s\n", num_chars, start, num_chars, start);

                // go directly to cursor which is '"', the increment will skip it and go
                // to next character
                current = cursor;
            }
        }
        break;

        default:
            if (isdigit(*current))
            {
                const char *start = current;
                char *cursor = NULL;
                double value = strtod(start, &cursor);
                int len = (int)(cursor - start);

                if (value == (long)value)
                {
                    printf("NUMBER %.*s %.1f\n", len, start, value);
                }
                else
                {
                    int format_len = len;
                    for (int i = len - 1; i >= 0; ++i)
                    {
                        if (start[i] != '0')
                            break;
                        else
                            --format_len;
                    }
                    printf("NUMBER %.*s %.*s\n", len, start, format_len, start);
                }

                current = cursor - 1;
            }
            else if (*current == '_' || isalpha(*current))
            {
                const char *start = current;
                const char *cursor = advance_while(start, alpha_or_underscore_or_digit);
                int len = (int)(cursor - start);
                if (!print_keyword(start, (size_t)len))
                {
                    printf("IDENTIFIER %.*s null\n", len, start);
                }
                current = cursor - 1;
            }
            else
            {
                fprintf(stderr, "[line %d] Error: Unexpected character: %c\n", line,
                        *current);
                retcode = 65;
            }
            break;
        }

        ++current;
    }

    printf("EOF  null\n");

    return retcode;
}
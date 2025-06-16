#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TOKEN_LIST   \
    X(UNKNOWN)       \
    X(UNTERMINATED)  \
    X(IDENTIFIER)    \
    X(STRING)        \
    X(NUMBER)        \
    X(IGNORE)        \
    X(LEFT_PAREN)    \
    X(RIGHT_PAREN)   \
    X(LEFT_BRACE)    \
    X(RIGHT_BRACE)   \
    X(COMMA)         \
    X(DOT)           \
    X(MINUS)         \
    X(PLUS)          \
    X(SEMICOLON)     \
    X(STAR)          \
    X(SLASH)         \
    X(EQUAL)         \
    X(EQUAL_EQUAL)   \
    X(BANG)          \
    X(BANG_EQUAL)    \
    X(LESS)          \
    X(LESS_EQUAL)    \
    X(GREATER)       \
    X(GREATER_EQUAL) \
    X(AND)           \
    X(CLASS)         \
    X(ELSE)          \
    X(FALSE)         \
    X(FOR)           \
    X(FUN)           \
    X(IF)            \
    X(NIL)           \
    X(OR)            \
    X(PRINT)         \
    X(RETURN)        \
    X(SUPER)         \
    X(THIS)          \
    X(TRUE)          \
    X(VAR)           \
    X(WHILE)

#define FOR_1_CHAR_TOKENS(DO) \
    DO('(', LEFT_PAREN)       \
    DO(')', RIGHT_PAREN)      \
    DO('{', LEFT_BRACE)       \
    DO('}', RIGHT_BRACE)      \
    DO(',', COMMA)            \
    DO('.', DOT)              \
    DO('-', MINUS)            \
    DO('+', PLUS)             \
    DO(';', SEMICOLON)        \
    DO('*', STAR)             \
    DO('/', SLASH)            \
    DO('=', EQUAL)            \
    DO('!', BANG)             \
    DO('<', LESS)             \
    DO('>', GREATER)

#define TWOCC(A, B) ((((unsigned)A) << 8) | (B))
#define FOR_2_CHAR_TOKENS(DO)        \
    DO(TWOCC('=', '='), EQUAL_EQUAL) \
    DO(TWOCC('!', '='), BANG_EQUAL)  \
    DO(TWOCC('<', '='), LESS_EQUAL)  \
    DO(TWOCC('>', '='), GREATER_EQUAL)

typedef struct
{
    const char *data;
    unsigned length;
} string_view_t;

typedef struct
{
    const char *start;
    const char *at;
    const char *end;
    unsigned line;
} tokenizer_t;

typedef enum
{
#define X(v) v,
    TOKEN_LIST
#undef X
        END_OF_FILE,
} token_kind_t;

typedef struct
{
    unsigned line;
    token_kind_t kind;
    string_view_t lexeme;
    union
    {
        string_view_t string;
        struct
        {
            long whole;
            long decimal;
        } number;
    } literal;
} token_t;

struct
{
    token_kind_t kind;
    const char *data;
    unsigned length;
} keywords[] = {
#define KWD(K, N) \
    {K, N, sizeof(N) - 1}
    KWD(AND, "and"),
    KWD(CLASS, "class"),
    KWD(ELSE, "else"),
    KWD(FALSE, "false"),
    KWD(FOR, "for"),
    KWD(FUN, "fun"),
    KWD(IF, "if"),
    KWD(NIL, "nil"),
    KWD(OR, "or"),
    KWD(PRINT, "print"),
    KWD(RETURN, "return"),
    KWD(SUPER, "super"),
    KWD(THIS, "this"),
    KWD(TRUE, "true"),
    KWD(VAR, "var"),
    KWD(WHILE, "while"),
#undef KWD
};
size_t n_keywords = sizeof(keywords) / sizeof(*keywords);

const char *kind2str[] = {
#define X(v) #v,
    TOKEN_LIST
#undef X
    "EOF",
};

char *read_file_contents(const char *filename);
void get_next_token(tokenizer_t *tokenizer, token_t *token);
void print_token(token_t *token);

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
        tokenizer_t tokenizer = {
            file_contents,
            file_contents,
            file_contents + strlen(file_contents),
            1,
        };

        int ret = 0;
        while (1)
        {
            token_t token;
            get_next_token(&tokenizer, &token);
            print_token(&token);
            if (token.kind == UNKNOWN || token.kind == UNTERMINATED)
                ret = 65;
            if (token.kind == END_OF_FILE)
                break;
        }

        free(file_contents);
        return ret;
    }
    else if (strcmp(command, "parse") == 0)
    {
        char *file_contents = read_file_contents(argv[2]);
        tokenizer_t tokenizer = {
            file_contents,
            file_contents,
            file_contents + strlen(file_contents),
            1,
        };

        while (1)
        {
            token_t token;
            get_next_token(&tokenizer, &token);
            switch (token.kind)
            {
            case NIL:
            case TRUE:
            case FALSE:
                printf("%.*s\n", token.lexeme.length, token.lexeme.data);
                break;
            default:
                break;
            };
            if (token.kind == END_OF_FILE)
                break;
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

static inline int at_eof(tokenizer_t *tok) { return tok->at >= tok->end; }

static inline int is_newline(char c) { return c == '\r' || c == '\n'; }

static inline int is_space(char c) { return c == ' ' || c == '\t'; }

static inline int is_number(char c) { return c >= '0' && c <= '9'; }

static inline int is_alpha(char c)
{
    return c == '_' || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static inline void strip_whitespace(tokenizer_t *tok)
{
    while (!at_eof(tok) && is_space(*tok->at))
        tok->at++;
}

void get_next_token(tokenizer_t *tokenizer, token_t *token)
{
    strip_whitespace(tokenizer);
    token->line = tokenizer->line;
    token->lexeme.data = tokenizer->at;
    token->lexeme.length = 0;

    if (at_eof(tokenizer))
    {
        token->kind = END_OF_FILE;
        return;
    }

    char a = *(tokenizer->at + 0);
    char b = *(tokenizer->at + 1);
    unsigned twocc = TWOCC(a, b);
    token->kind = UNKNOWN;

    if (is_alpha(a))
    {
        token->kind = IDENTIFIER;
        while (is_alpha(*tokenizer->at) || is_number(*tokenizer->at))
            tokenizer->at++;
        token->lexeme.length = tokenizer->at - token->lexeme.data;
        const char *d = token->lexeme.data;
        unsigned l = token->lexeme.length;
        for (size_t i = 0; i < n_keywords; i++)
        {
            if (keywords[i].length == l && !memcmp(d, keywords[i].data, l))
            {
                token->kind = keywords[i].kind;
                return;
            }
        }
        return;
    }

    if (is_number(a))
    {
        char *end;
        token->kind = NUMBER;
        token->literal.number.whole = strtol(tokenizer->at, &end, 10);
        token->literal.number.decimal = 0;
        if (*end == '.' && is_number(*(end + 1)))
        {
            long dec = strtol(end + 1, &end, 10);
            for (; dec && dec % 10 == 0; dec /= 10)
                ;
            token->literal.number.decimal = dec;
        }
        tokenizer->at = end;
        token->lexeme.length = tokenizer->at - token->lexeme.data;
        return;
    }

    if (a == '"')
    {
        for (tokenizer->at += 1; *tokenizer->at != '"'; tokenizer->at++)
        {
            if (at_eof(tokenizer) || is_newline(*tokenizer->at))
            {
                token->kind = UNTERMINATED;
                return;
            }
        }
        tokenizer->at += 1;
        token->kind = STRING;
        token->lexeme.length = tokenizer->at - token->lexeme.data;
        token->literal.string.data = token->lexeme.data + 1;
        token->literal.string.length = token->lexeme.length - 2;
        return;
    }

    if (twocc == TWOCC('/', '/'))
    {
        while (!at_eof(tokenizer) && !is_newline(*tokenizer->at))
            tokenizer->at++;
        tokenizer->at += *tokenizer->at == '\r';
        tokenizer->at += *tokenizer->at == '\n';
        tokenizer->line += 1;
        token->kind = IGNORE;
        return;
    }

    switch (twocc)
    {
#define DO(S, T)         \
    case S:              \
        token->kind = T; \
        break;
        FOR_2_CHAR_TOKENS(DO)
#undef DO
    }

    if (token->kind != UNKNOWN)
    {
        token->lexeme.length = 2;
        tokenizer->at += 2;
        return;
    }

    switch (a)
    {
#define DO(S, T)         \
    case S:              \
        token->kind = T; \
        break;
        FOR_1_CHAR_TOKENS(DO)
#undef DO
    case '\r':
        tokenizer->at += b == '\n';
    case '\n':
        tokenizer->line += 1;
        token->kind = IGNORE;
        break;
    }

    token->lexeme.length = 1;
    tokenizer->at += 1;
}

void print_token(token_t *token)
{
    if (token->kind == UNKNOWN)
    {
        fprintf(stderr, "[line %d] Error: Unexpected character: %c\n", token->line,
                token->lexeme.data[0]);
    }
    else if (token->kind == UNTERMINATED)
    {
        fprintf(stderr, "[line %d] Error: Unterminated string.\n", token->line);
    }
    else if (token->kind != IGNORE)
    {
        fprintf(stdout, "%s %.*s ", kind2str[token->kind], token->lexeme.length,
                token->lexeme.data);
        switch (token->kind)
        {
        case STRING:
            fprintf(stdout, "%.*s\n", token->literal.string.length,
                    token->literal.string.data);
            break;
        case NUMBER:
            fprintf(stdout, "%ld.%ld\n", token->literal.number.whole,
                    token->literal.number.decimal);
            break;
        default:
            fprintf(stdout, "null\n");
            break;
        }
    }
}

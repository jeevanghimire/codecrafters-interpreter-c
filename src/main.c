#include <assert.h>
#include <complex.h>
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LENGTH_OF(x) sizeof(x) / sizeof(*x)
#define ARGS_SLICE(x) x.count, x.data

#define true 1
#define false 0
typedef int boolean;

typedef struct
{
    char *data;
    int count;
} Slice;

typedef enum
{
    KIND_INVALID = 0,
    KIND_OPERANT,
    KIND_UNTERMS,
    KIND_STRING,
    KIND_NUMBER,
    KIND_IDENT,
    KIND_EOF,
    KIND_RW,
} TokenKind;

typedef enum
{
    TYPE_LEFT_PAREN = 0,
    TYPE_RIGHT_PAREN,
    TYPE_LEFT_BRACE,
    TYPE_RIGHT_BRACE,
    TYPE_COMMA,
    TYPE_DOT,
    TYPE_MINUS,
    TYPE_PLUS,
    TYPE_SEMICOLON,
    TYPE_SLASH,
    TYPE_STAR,
    TYPE_EQUAL_EQUAL,
    TYPE_EQUAL,
    TYPE_BANG_EQUAL,
    TYPE_BANG,
    TYPE_LESS_EQUAL,
    TYPE_LESS,
    TYPE_GREATER_EQUAL,
    TYPE_GREATER,
    TYPE_AND,
    TYPE_CLASS,
    TYPE_ELSE,
    TYPE_FALSE,
    TYPE_FOR,
    TYPE_FUN,
    TYPE_IF,
    TYPE_NIL,
    TYPE_OR,
    TYPE_PRINT,
    TYPE_RETURN,
    TYPE_SUPER,
    TYPE_THIS,
    TYPE_TRUE,
    TYPE_VAR,
    TYPE_WHILE,
    TYPE_NUMBER,
    TYPE_IDENT,
    TYPE_STRING,
    TYPE_EOF,
} TokenType;

char *TYPE_NAMES[] = {"LEFT_PAREN", "RIGHT_PAREN", "LEFT_BRACE",
                      "RIGHT_BRACE", "COMMA", "DOT",
                      "MINUS", "PLUS", "SEMICOLON",
                      "SLASH", "STAR", "EQUAL_EQUAL",
                      "EQUAL", "BANG_EQUAL", "BANG",
                      "LESS_EQUAL", "LESS", "GREATER_EQUAL",
                      "GREATER", "AND", "CLASS",
                      "ELSE", "FALSE", "FOR",
                      "FUN", "IF", "NIL",
                      "OR", "PRINT", "RETURN",
                      "SUPER", "THIS", "TRUE",
                      "VAR", "WHILE", "NUMBER",
                      "IDENTIFIER", "STRING", "EOF"};

char *OPERANTS[] = {"(", ")", "{", "}", ",", ".", "-", "+", ";", "/",
                    "*", "==", "=", "!=", "!", "<=", "<", ">=", ">"};
const size_t OPERANTS_BEGIN = 0;
const size_t OPERANTS_COUNT = LENGTH_OF(OPERANTS);

char *RESERVED[] = {"AND", "CLASS", "ELSE", "FALSE", "FOR", "FUN",
                    "IF", "NIL", "OR", "PRINT", "RETURN", "SUPER",
                    "THIS", "TRUE", "VAR", "WHILE"};
const size_t RESERVED_BEGIN = OPERANTS_BEGIN + OPERANTS_COUNT;
const size_t RESERVED_COUNT = LENGTH_OF(RESERVED);

typedef union
{
    double number;
    boolean boolean;
} TokenData;

typedef struct
{
    TokenKind kind;
    TokenType type;
    Slice source;
    TokenData as;
} Token;

typedef struct
{
    Token *data;
    size_t count, cap;
} Tokens;

size_t append_token(Tokens *tokens, Token token)
{
    if (tokens->count == tokens->cap)
    {
        if (!tokens->cap)
            tokens->cap = 8;
        else
            tokens->cap *= 2;
        tokens->data = realloc(tokens->data, tokens->cap * sizeof(Token));
    }
    tokens->data[tokens->count] = token;
    return tokens->count++;
}

typedef struct
{
    int exit_code;
    size_t line;
} Context;

char *read_file_contents(const char *filename);

size_t dist(char *start, char *end) { return end - start; }

Slice expand_to(Slice *slice, char *to)
{
    slice->count = dist(slice->data, to);
    return *slice;
}

char char_at(char *cursor) { return *cursor; }
char char_off(char *cursor, int off) { return *(cursor + off); }

boolean is_eof(char c) { return c == '\0'; }
boolean is_eol(char c) { return c == '\n' || is_eof(c); }
boolean is_white_space(int c) { return c == ' ' || c == '\n' || c == '\t'; }
boolean is_ident_start(int c) { return isalpha(c) || c == '_'; }
boolean is_ident(int c) { return is_ident_start(c) || isdigit(c); }

char next(char **cursor)
{
    char c = **cursor;
    ++*cursor;
    return c;
}
char prev(char **cursor)
{
    char c = char_at(*cursor);
    --*cursor;
    return c;
}

/*
  Seeks forward in the string until it finds `match`.
  Returns true if found or false if reached end of file/string.
  `cursor` will sit on the match char, or on the char indicating eof.
*/
boolean find(char **cursor, const char match)
{
    while (**cursor != match && !is_eof(**cursor))
        next(cursor);
    return !is_eof(**cursor);
}

/*
  Seeks forward in the line until it finds `match`.
  Returns true if found or false if reached end of line.
  `cursor` will sit on the match char, or on the char indicating eol.
*/
boolean find_in_line(char **cursor, const char match)
{
    while (char_at(*cursor) != match && !is_eol(**cursor))
        next(cursor);
    return !is_eol(**cursor);
}

/*
  Matches the char under the cursor if f(char) != 0.
  On match moves the cursor forward.
  Returns boolean whether matched.
*/
boolean match_f(char **cursor, boolean (*f)(int))
{
    if (!f(**cursor))
        return false;
    next(cursor);
    return true;
}

/*
  Matches the char under the cursor with the given match.
  On match moves the cursor forward.
  Returns boolean whether matched.
*/
boolean match(char **cursor, const char match)
{
    if (char_at(*cursor) != match)
        return false;
    next(cursor);
    return true;
}

boolean match_cstr(char **cursor, char *cstr)
{
    char *l_cursor = *cursor;
    for (int i = 0; i < strlen(cstr); ++i)
        if (!match(&l_cursor, cstr[i]))
            return false;
    *cursor = l_cursor;
    return true;
}

boolean match_lower_cstr(char **cursor, char *cstr)
{
    char *l_cursor = *cursor;
    for (int i = 0; i < strlen(cstr); ++i)
    {
        int r = match(&l_cursor, tolower(cstr[i]));
        if (!r)
            return false;
    }
    *cursor = l_cursor;
    return true;
}

void seek_last_c_in_line(char **cursor)
{
    find_in_line(cursor, '\n');
    prev(cursor);
}

/*
  Matches all chars after the cursor where f(char) != 0.
  Cursor sits on the char after the last match.
*/
void match_while_f(char **cursor, int (*f)(int))
{
    while (match_f(cursor, f))
        ;
}

int match_one_of(char **cursor, char **list, size_t len, int list_to_lower)
{
    for (int i = 0; i < len; ++i)
        if ((list_to_lower && match_cstr(cursor, list[i])) ||
            match_lower_cstr(cursor, list[i]))
            return i;
    return -1;
}

/*
  Compares the string slice at cursor to the cstr (null terminated).
  Returns whether equal.
*/
boolean str_eq(Slice slice, char *cstr)
{
    if (strlen(cstr) != slice.count)
        return false;
    for (int i = 0; i < slice.count; ++i)
        if (slice.data[i] != tolower(cstr[i]))
            return false;
    return true;
}

int contains_str(char *list[], size_t len, Slice string)
{
    for (int i = 0; i < len; ++i)
        if (str_eq(string, list[i]))
            return i;
    return -1;
}

int get_reserved_type(Slice slice)
{
    int index = contains_str(RESERVED, RESERVED_COUNT, slice);
    if (index == -1)
        return -1;
    return RESERVED_BEGIN + index;
}

char *fmt_number(double number)
{
    // Numbers are suppost to printed with at minimum one decimal digit and as
    // many as there are. printf("%f") can only print a fix number of decimal
    // places. So we need to strip of some '0' from the end.

    // Print full number in buffer
    static char buffer[64];
    sprintf(buffer, "%f", number);
    // Seek to the last '0' (one before the null char).
    char *p = buffer;
    seek_last_c_in_line(&p);
    // Replace all '0' with null chars.
    while (prev(&p) == '0')
        p[1] = '\0';
    // If the last char is now a decimal point, add one '0' back in.
    if (p[1] == '.')
        p[2] = '0';
    return buffer;
}

void print_number(Slice slice, double number)
{
    printf("NUMBER %.*s %s\n", slice.count, slice.data, fmt_number(number));
}

void print_rw(char *rw_cstr, Slice slice)
{
    assert(slice.count <= 6 && "longest reserved word is 'return'");
    printf("%s %.*s null\n", rw_cstr, slice.count, slice.data);
}
void print_ident(Slice slice)
{
    printf("IDENTIFIER %.*s null\n", slice.count, slice.data);
}
void print_string(Slice slice)
{
    printf("STRING \"%.*s\" %.*s\n", slice.count, slice.data, slice.count,
           slice.data);
}
void print_operant(char *name, Slice slice)
{
    printf("%s %.*s null\n", name, slice.count, slice.data);
}
void print_token(Token token)
{
    switch (token.kind)
    {
    case KIND_OPERANT:
        print_operant(TYPE_NAMES[token.type], token.source);
        break;
    case KIND_STRING:
        print_string(token.source);
        break;
    case KIND_NUMBER:
        print_number(token.source, token.as.number);
        break;
    case KIND_IDENT:
        print_ident(token.source);
        break;
    case KIND_RW:
        print_rw(TYPE_NAMES[token.type], token.source);
        break;
    case KIND_UNTERMS:
        break;
    case KIND_EOF:
        printf("EOF  null\n");
        break;
    case KIND_INVALID:
        assert(0 && "Can't print invalid token!");
        break;
    }
}

boolean next_operant(char **cursor, Token *token)
{
    token->source.data = *cursor;
    int index = match_one_of(cursor, OPERANTS, LENGTH_OF(OPERANTS), false);
    if (index < 0)
        return false;
    token->kind = KIND_OPERANT;
    token->type = (TokenType)index;
    expand_to(&token->source, *cursor);
    return true;
}

boolean next_string(char **cursor, Token *out_token, Context *ctx)
{
    if (**cursor != '"')
        return false;
    next(cursor);
    out_token->kind = KIND_STRING;
    out_token->type = TYPE_STRING;
    out_token->source.data = *cursor;
    if (!find_in_line(cursor, '"'))
    {
        fprintf(stderr, "[line %zu] Error: Unterminated string.\n", ctx->line);
        out_token->kind = KIND_UNTERMS;
        ctx->exit_code = 65;
        prev(cursor); // Cancle out next() below
    }
    expand_to(&out_token->source, *cursor);
    next(cursor); // Move over closing '"'.
    return true;
}

boolean next_number(char **cursor, Token *out_token)
{
    if (!isdigit(**cursor))
        return false;
    out_token->kind = KIND_NUMBER;
    out_token->type = TYPE_NUMBER;
    out_token->source.data = *cursor;
    out_token->as.number = strtod(*cursor, cursor);
    expand_to(&out_token->source, *cursor);
    return true;
}

boolean next_ident(char **cursor, Token *out_token)
{
    if (!is_ident_start(**cursor))
        return false;
    out_token->source.data = *cursor;
    match_while_f(cursor, is_ident);
    expand_to(&out_token->source, *cursor);
    out_token->type = get_reserved_type(out_token->source);
    if (out_token->type != -1)
    {
        out_token->kind = KIND_RW;
    }
    else
    {
        out_token->kind = KIND_IDENT;
        out_token->type = TYPE_IDENT;
    }
    return true;
}

boolean next_token(char **cursor, Token *out_token, Context *ctx)
{
    return next_number(cursor, out_token) || next_ident(cursor, out_token) ||
           next_string(cursor, out_token, ctx) || next_operant(cursor, out_token);
}

void tokenize(char *contents, Tokens *tokens, Context *ctx)
{
    for (char *cursor = contents; !is_eof(*cursor);)
    {
        Token token = {0};
        if (match(&cursor, '\n'))
        {
            ++ctx->line;
        }
        else if (match_cstr(&cursor, "//"))
        {
            find(&cursor, '\n');
        }
        else if (match_f(&cursor, is_white_space))
        {
        }
        else if (next_token(&cursor, &token, ctx))
        {
            append_token(tokens, token);
        }
        else
        {
            fprintf(stderr, "[line %zu] Error: Unexpected character: %c\n", ctx->line,
                    next(&cursor));
            ctx->exit_code = 65;
        }
    }
    Token eof = {.kind = KIND_EOF};
    append_token(tokens, eof);
}

int main(int argc, char *argv[])
{
    // Disable output buffering
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
    Context ctx = {.line = 1, .exit_code = 0};

    if (argc < 3)
    {
        fprintf(stderr, "Usage: ./your_program tokenize <filename>\n");
        return 1;
    }

    char *contents = read_file_contents(argv[2]);
    if (!contents)
        return 1;

    const char *command = argv[1];
    if (strcmp(command, "tokenize") == 0)
    {
        Tokens tokens = {0};
        tokenize(contents, &tokens, &ctx);
        for (int i = 0; i < tokens.count; ++i)
            print_token(tokens.data[i]);
        free(tokens.data);
    }
    else if (strcmp(command, "parse") == 0)
    {
        Tokens tokens = {0};
        tokenize(contents, &tokens, &ctx);
        for (int i = 0; i < tokens.count; ++i)
        {
            switch (tokens.data[i].kind)
            {
            case KIND_INVALID:
            case KIND_UNTERMS:
                assert(false && "unimplemented");
            case KIND_NUMBER:
                printf("%s", fmt_number(tokens.data[i].as.number));
                break;
            case KIND_OPERANT:
            case KIND_STRING:
            case KIND_IDENT:
            case KIND_RW:
                printf("%.*s", ARGS_SLICE(tokens.data[i].source));
                break;
            case KIND_EOF:
                break;
            }
        }
        free(tokens.data);
    }
    else
    {
        fprintf(stderr, "Unknown command: %s\n", command);
        ctx.exit_code = 1;
    }

    free(contents);
    return ctx.exit_code;
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
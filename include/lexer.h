#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>

typedef enum {
    TOK_TEXT,
    TOK_COMMAND,
    TOK_ARGUMENT,
    TOK_END,
    TOK_INVALID
} TokenType;

typedef struct {
    TokenType type;
    const char *start;
    size_t length;
} Token;

typedef struct {
    const char *input;
    size_t position;
    size_t length;
    int after_command;
} Lexer;

void lexer_init(Lexer *lexer, const char *input);
Token lexer_next(Lexer *lexer);

#endif

#include "lexer.h"
#include <string.h>

void lexer_init(Lexer *lexer, const char *input)
{
    lexer->input = input;
    lexer->position = 0;
    lexer->length = strlen(input);
    lexer->after_command = 0;
}

Token lexer_next(Lexer *lexer)
{
    Token token;

    while (lexer->position < lexer->length &&
           lexer->input[lexer->position] == ' ') {
        lexer->position++;
    }

    if (lexer->position >= lexer->length ||
        lexer->input[lexer->position] == '\n') {

        token.type = TOK_END;
        token.start = NULL;
        token.length = 0;

        return token;
    }

    size_t start = lexer->position;

    if (lexer->position == 0 &&
        lexer->input[lexer->position] == '/') {

        while (lexer->position < lexer->length &&
               lexer->input[lexer->position] != ' ' &&
               lexer->input[lexer->position] != '\n') {

            lexer->position++;
        }

        token.type = TOK_COMMAND;
        token.start = lexer->input + start;
        token.length = lexer->position - start;

        lexer->after_command = 1;

        return token;
    }

    if (lexer->after_command) {

        while (lexer->position < lexer->length &&
               lexer->input[lexer->position] != '\n') {

            lexer->position++;
        }

        token.type = TOK_ARGUMENT;
        token.start = lexer->input + start;
        token.length = lexer->position - start;

        return token;
    }

    while (lexer->position < lexer->length &&
           lexer->input[lexer->position] != '\n') {

        lexer->position++;
    }

    token.type = TOK_TEXT;
    token.start = lexer->input + start;
    token.length = lexer->position - start;

    return token;
}

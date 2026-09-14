#include <stdio.h>

#include "token_debug.h"

void print_token(Token token)
{
    switch (token.type) {

        case TOK_TEXT:
            printf("TEXT: %.*s\n",
                   (int)token.length,
                   token.start);
            break;

        case TOK_COMMAND:
            printf("COMMAND: %.*s\n",
                   (int)token.length,
                   token.start);
            break;

        case TOK_ARGUMENT:
            printf("ARGUMENT: %.*s\n",
                   (int)token.length,
                   token.start);
            break;

        case TOK_END:
            printf("END\n");
            break;

        case TOK_INVALID:
            printf("INVALID\n");
            break;
    }
}

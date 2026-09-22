#include "parser.h"

#include <string.h>


static int token_equals(
    Token token,
    const char *text
)
{
    size_t text_length = strlen(text);

    if (token.length != text_length) {
        return 0;
    }

    return strncmp(
        token.start,
        text,
        token.length
    ) == 0;
}
Command parse_command(Lexer *lexer)
{
    Command command;

    command.type = CMD_INVALID;
    command.argument = NULL;
    command.argument_length = 0;

    Token first = lexer_next(lexer);

    /*
     * Mensagem comum.
     */
    if (first.type == TOK_TEXT) {

        command.type = CMD_TEXT;
        command.argument = first.start;
        command.argument_length = first.length;

        return command;
    }

    /*
     * Esperávamos texto ou comando.
     */
    if (first.type != TOK_COMMAND) {
        return command;
    }

    /*
     * /name
     */
    if (token_equals(first, "/name")) {

        Token argument = lexer_next(lexer);

        if (argument.type != TOK_ARGUMENT) {
            return command;
        }

        command.type = CMD_NAME;
        command.argument = argument.start;
        command.argument_length = argument.length;

        return command;
    }

    /*
     * /quit
     */
    if (token_equals(first, "/quit")) {

        command.type = CMD_QUIT;

        return command;
    }

    return command;
}

#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef enum {
    CMD_TEXT,
    CMD_NAME,
    CMD_QUIT,
    CMD_INVALID
} CommandType;

typedef struct {
    CommandType type;

    const char *argument;
    size_t argument_length;

} Command;

Command parse_command(Lexer *lexer);

#endif

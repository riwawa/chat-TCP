#include "screen.h"

#include <stdio.h>
#include <string.h>


void screen_clear(void)
{
    /*
     * ANSI escape codes:
     *
     * \033[2J
     * limpa a tela
     *
     * \033[H
     * move cursor para topo esquerdo
     */
    printf("\033[2J");
    printf("\033[H");
}


void screen_init(Screen *screen)
{
    if (screen == NULL) return;
    
    memset(screen, 0, sizeof(*screen));

    strcpy(
        screen->username,
        "anonymous"
    );

    strcpy(
        screen->room,
        "general"
    );

    screen->connected = 0;
}


void screen_set_username(
    Screen *screen,
    const char *username
)
{
    if (
        screen == NULL ||
        username == NULL
    ) {
        return;
    }

    strncpy(
        screen->username,
        username,
        sizeof(screen->username) - 1
    );

    screen->username[
        sizeof(screen->username) - 1
    ] = '\0';
}


void screen_set_room(
    Screen *screen,
    const char *room
)
{
    if (
        screen == NULL ||
        room == NULL
    ) {
        return;
    }

    strncpy(
        screen->room,
        room,
        sizeof(screen->room) - 1
    );

    screen->room[
        sizeof(screen->room) - 1
    ] = '\0';
}


void screen_set_connected(
    Screen *screen,
    int connected
)
{
    if (screen == NULL) {
        return;
    }

    screen->connected = connected;
}


void screen_add_message(
    Screen *screen,
    const char *message
)
{
    if (
        screen == NULL ||
        message == NULL
    ) {
        return;
    }
    if (
        screen->count <
        SCREEN_HISTORY_SIZE
    ) {

        strncpy(
            screen->messages[
                screen->count
            ],
            message,
            SCREEN_MESSAGE_SIZE - 1
        );

        screen->messages[
            screen->count
        ][SCREEN_MESSAGE_SIZE - 1] = '\0';

        screen->count++;

        return;
    }

    for (
        int i = 1;
        i < SCREEN_HISTORY_SIZE;
        i++
    ) {

        strcpy(
            screen->messages[i - 1],
            screen->messages[i]
        );
    }

    strncpy(
        screen->messages[
            SCREEN_HISTORY_SIZE - 1
        ],
        message,
        SCREEN_MESSAGE_SIZE - 1
    );

    screen->messages[
        SCREEN_HISTORY_SIZE - 1
    ][SCREEN_MESSAGE_SIZE - 1] = '\0';
}


void screen_render(
    const Screen *screen
)
{
    if (screen == NULL) {
        return;
    }

    screen_clear();

    printf(
        "╭────────────────────────────────────────────────────────────╮\n"
    );

    printf(
        "│                         TCP CHAT                           │\n"
    );

    printf(
        "│  %s • user: %-15s • room: %-15s │\n",
        screen->connected
            ? "connected   "
            : "disconnected",
        screen->username,
        screen->room
    );

    printf(
        "├────────────────────────────────────────────────────────────┤\n"
    );

    printf(
        "│                                                            │\n"
    );


    int visible =
        screen->count;

    if (
        visible >
        SCREEN_HISTORY_SIZE
    ) {
        visible =
            SCREEN_HISTORY_SIZE;
    }


    for (
        int i = 0;
        i < visible;
        i++
    ) {

        char line[
            SCREEN_MESSAGE_SIZE
        ];

        strncpy(
            line,
            screen->messages[i],
            sizeof(line) - 1
        );

        line[
            sizeof(line) - 1
        ] = '\0';

        /*
         * remove \n para não quebrar
         * o layout da caixa
         */
        line[
            strcspn(line, "\n")
        ] = '\0';

        printf(
            "│  %-58.58s│\n",
            line
        );
    }

    for (
        int i = visible;
        i < 12;
        i++
    ) {

        printf(
            "│                                                            │\n"
        );
    }


    printf(
        "├────────────────────────────────────────────────────────────┤\n"
    );

    printf(
        "│ >                                                          │\n"
    );

    printf(
        "╰────────────────────────────────────────────────────────────╯\n"
    );

    printf(
        "\033[2A"
        "\033[4C"
    );

    fflush(stdout);
}

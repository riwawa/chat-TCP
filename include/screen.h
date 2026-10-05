#ifndef SCREEN_H
#define SCREEN_H

#define SCREEN_HISTORY_SIZE 20
#define SCREEN_MESSAGE_SIZE 256

typedef struct {
    char messages[SCREEN_HISTORY_SIZE][SCREEN_MESSAGE_SIZE];
    int count;
    char username[32];
    char room[32];

    int connected;
} Screen;

void screen_init(Screen *screen);

void screen_set_username(
    Screen *screen,
    const char *username
);

void screen_set_room(
    Screen *screen,
    const char *room
);

void screen_set_connected(
    Screen *screen,
    int connected
);

void screen_add_message(
    Screen *screen,
    const char *message
);

void screen_render(
    const Screen *screen
);

void screen_clear(void);

#endif

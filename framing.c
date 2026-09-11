#include "framing.h"
#include <string.h>

int framing_append(FrameBuffer *fb,
                   const char *data,
                   size_t len)
{
    if (fb->used + len > sizeof(fb->data)) {
        return -1;
    }
    memcpy(fb->data + fb->used, data, len);
    fb->used += len;

    return 0;
}

int framing_extract(FrameBuffer *fb,
                    char *message,
                    size_t message_size)
{
    char *newline =
        memchr(fb->data, '\n', fb->used);
    if (newline == NULL) {
        return 0;
    }
    size_t message_len =
        (size_t)(newline - fb->data) + 1;

    if (message_len >= message_size) {
        return -1;
    }

    memcpy(message, fb->data, message_len);
    message[message_len] = '\0';

    size_t remaining =
        fb->used - message_len;

    memmove(fb->data,
            fb->data + message_len,
            remaining);

    fb->used = remaining;

    return 1;
}
#ifndef FRAMING_H
#define FRAMING_H

#include <stddef.h>

typedef struct {
    char data[1024];
    size_t used;
} FrameBuffer;

int framing_append(FrameBuffer *fb,
                   const char *data,
                   size_t len);

int framing_extract(FrameBuffer *fb,
                    char *message,
                    size_t message_size);

#endif
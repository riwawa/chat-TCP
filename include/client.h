#ifndef CLIENT_H
#define CLIENT_H

#include "framing.h"

typedef struct {
    int fd;
    FrameBuffer frame_buffer;
} Client;

#endif
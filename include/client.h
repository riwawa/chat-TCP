#ifndef CLIENT_H
#define CLIENT_H

#include "framing.h"

typedef struct {
    int fd;
    FrameBuffer frame_buffer;

    char username[32];
    int authenticated;

} Client;

#endif

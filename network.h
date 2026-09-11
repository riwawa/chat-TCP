#ifndef NETWORK_H
#define NETWORK_H

#include <stddef.h>
#include <sys/types.h>

int create_tcp_socket(void);

int send_all(int socket_fd,
             const char *buffer,
             size_t length);

ssize_t receive_bytes(int socket_fd,
                      void *buffer,
                      size_t size);

void close_connection(int socket_fd);

#endif
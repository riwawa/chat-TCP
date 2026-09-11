#include "network.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>


int create_tcp_socket(void)
{
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        return -1;
    }
    return socket_fd;
}


int send_all(int socket_fd,
             const char *buffer,
             size_t length)
{
    size_t total = 0;
    while (total < length) {
        ssize_t n = send(socket_fd,
                         buffer + total,
                         length - total,
                         0);

        if (n <= 0) {
            return -1;
        }
        total += (size_t)n;
    }

    return 0;
}


ssize_t receive_bytes(int socket_fd,
                      void *buffer,
                      size_t size)
{
    return recv(socket_fd,
                buffer,
                size,
                0);
}


void close_connection(int socket_fd)
{
    close(socket_fd);
}
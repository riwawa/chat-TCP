#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <unistd.h>

#include "network.h"
#include "framing.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9002

#define TEMP_BUFFER_SIZE 256
#define MESSAGE_SIZE 256


void setup_pollfds(
    struct pollfd fds[],
    int socket_fd
)
{
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    fds[0].revents = 0;
    fds[1].fd = socket_fd;
    fds[1].events = POLLIN;
    fds[1].revents = 0;
}


int connect_to_server(
    const char *ip,
    int port
)
{
    int fd = create_tcp_socket();

    if (fd < 0) {
        return -1;
    }

    struct sockaddr_in server_address = {0};

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);

    if (inet_pton(
            AF_INET,
            ip,
            &server_address.sin_addr
        ) <= 0) {

        close_connection(fd);
        return -1;
    }

    if (connect(
            fd,
            (struct sockaddr *)&server_address,
            sizeof(server_address)
        ) < 0) {

        close_connection(fd);
        return -1;
    }

    return fd;
}


int main(void)
{
    int server_fd = connect_to_server(
        SERVER_IP,
        SERVER_PORT
    );

    if (server_fd < 0) {

        printf(
            "Erro ao conectar ao servidor\n"
        );

        return EXIT_FAILURE;
    }

    printf(
        "Conectado ao servidor %s:%d\n",
        SERVER_IP,
        SERVER_PORT
    );

    struct pollfd fds[2];

    setup_pollfds(
        fds,
        server_fd
    );


    char user_input[MESSAGE_SIZE];
    char temp[TEMP_BUFFER_SIZE];
    char message[MESSAGE_SIZE];

    FrameBuffer frame_buffer = {0};


    while (1) {
        int ready = poll(
            fds,
            2,
            -1
        );

        if (ready < 0) {

            perror("poll");
            break;
        }

        if (fds[0].revents & POLLIN) {

            if (fgets(
                    user_input,
                    sizeof(user_input),
                    stdin
                ) == NULL) {

                printf(
                    "Entrada encerrada\n"
                );

                break;
            }

            if (send_all(
                    server_fd,
                    user_input,
                    strlen(user_input)
                ) < 0) {

                printf(
                    "Erro ao enviar mensagem\n"
                );

                break;
            }
        }

        if (fds[1].revents & POLLIN) {

            ssize_t n = receive_bytes(
                server_fd,
                temp,
                sizeof(temp)
            );

            if (n > 0) {
                if (framing_append(
                        &frame_buffer,
                        temp,
                        (size_t)n
                    ) < 0) {

                    printf(
                        "Erro de framing: "
                        "buffer cheio\n"
                    );

                    break;
                }

                while (1) {

                    int status =
                        framing_extract(
                            &frame_buffer,
                            message,
                            sizeof(message)
                        );

                    if (status < 0) {

                        printf(
                            "Erro ao extrair mensagem\n"
                        );

                        break;
                    }

                    if (status == 0) {
                        break;
                    }



                    printf(
                        "%s",
                        message
                    );
                }
            }


            /*
             * recv() == 0
             *
             * O servidor fechou a conexão.
             */
            else if (n == 0) {

                printf(
                    "Servidor desconectou\n"
                );

                break;
            }


            /*
             * recv() < 0
             */
            else {

                printf(
                    "Erro ao receber dados\n"
                );

                break;
            }
        }
    }


    close_connection(
        server_fd
    );

    return 0;
}
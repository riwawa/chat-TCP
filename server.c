#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include "network.h"

int start_server(int port)
{
    int server_fd = create_tcp_socket();

    if (server_fd < 0) {
        return -1;
    }

    int opt = 1;

    if (setsockopt(server_fd,
                   SOL_SOCKET,
                   SO_REUSEADDR,
                   &opt,
                   sizeof(opt)) < 0) {

        close(server_fd);
        return -1;
    }

    struct sockaddr_in endereco_servidor = {0};

    endereco_servidor.sin_family = AF_INET;
    endereco_servidor.sin_port = htons(port);
    endereco_servidor.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server_fd,
             (struct sockaddr *)&endereco_servidor,
             sizeof(endereco_servidor)) < 0) {

        close(server_fd);
        return -1;
    }

    if (listen(server_fd, 5) < 0) {
        close(server_fd);
        return -1;
    }

    return server_fd;
}
int accept_client(int server_fd)
{
    return accept(server_fd, NULL, NULL);
}


int main()
{
    int servidor_network = start_server(9002);
    if (servidor_network < 0) {
        printf("Erro ao iniciar servidor\n");
        exit(EXIT_FAILURE);
    }

    int client_socket = accept_client(servidor_network);
    if (client_socket < 0) {
        printf("Couldn't accept client\n");
        close(servidor_network);
        exit(EXIT_FAILURE);
    }


    char server_message[256];
    char temp[256];
    char recv_buffer[1024];
    size_t recv_used = 0;

    while (1) {

        /*
         * SERVER -> CLIENT
         */

        if (fgets(server_message,
                  sizeof(server_message),
                  stdin) == NULL) {
            break;
        }

        if (send_all(client_socket,
                     server_message,
                     strlen(server_message)) < 0) {

            printf("Erro ao enviar mensagem\n");
            break;
        }


        /*
         * CLIENT -> SERVER
         */

        ssize_t n = recv(client_socket,
                         temp,
                         sizeof(temp),
                         0);

        if (n > 0) {
            if (recv_used + (size_t)n > sizeof(recv_buffer)) {
                printf("Buffer cheio: mensagem sem delimitador muito grande\n");
                break;
            }

            memcpy(recv_buffer + recv_used,
                   temp,
                   (size_t)n);

            recv_used += (size_t)n;
            char *newline;
            while ((newline =
                    memchr(recv_buffer,
                           '\n',
                           recv_used)) != NULL) {

                size_t message_len =
                    (size_t)(newline - recv_buffer) + 1;

                char message[256];
                if (message_len >= sizeof(message)) {
                    printf("Mensagem grande demais\n");
                    close(client_socket);
                    close(servidor_network);

                    return EXIT_FAILURE;
                }

                memcpy(message,
                       recv_buffer,
                       message_len);

                message[message_len] = '\0';


                if (strcmp(message, "/quit\n") == 0) {
                    printf("Cliente encerrou a conexão\n");
                    close(client_socket);
                    close(servidor_network);
                    return 0;
                }
                printf("Cliente: %s", message);

                size_t remaining =
                    recv_used - message_len;

                memmove(recv_buffer,
                        recv_buffer + message_len,
                        remaining);

                recv_used = remaining;
            }

        }
        else if (n == 0) {
            printf("Cliente desconectou\n");
            break;

        }
        else {
            printf("Erro no recv\n");
            break;
        }
    }


    close(client_socket);
    close(servidor_network);

    return 0;
}
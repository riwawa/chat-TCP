#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "network.h"
#include "framing.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9002
#define TEMP_BUFFER_SIZE 256
#define MESSAGE_SIZE 256


int connect_to_server(const char *ip, int port)
{
    int fd = create_tcp_socket();

    if (fd < 0) {
        return -1;
    }

    struct sockaddr_in endereco_servidor = {0};

    endereco_servidor.sin_family = AF_INET;
    endereco_servidor.sin_port = htons(port);

    if (inet_pton(
            AF_INET,
            ip,
            &endereco_servidor.sin_addr
        ) <= 0) {

        close_connection(fd);
        return -1;
    }

    if (connect(
            fd,
            (struct sockaddr *)&endereco_servidor,
            sizeof(endereco_servidor)
        ) < 0) {

        close_connection(fd);
        return -1;
    }

    return fd;
}


int main(void)
{
    int cliente_network =
        connect_to_server(
            SERVER_IP,
            SERVER_PORT
        );

    if (cliente_network < 0) {
        printf("Erro ao conectar ao servidor\n");
        return EXIT_FAILURE;
    }

    printf(
        "Cliente conectado a %s:%d\n",
        SERVER_IP,
        SERVER_PORT
    );

    char user_input[MESSAGE_SIZE];
    char temp[TEMP_BUFFER_SIZE];
    char message[MESSAGE_SIZE];

    FrameBuffer frame_buffer = {0};

    while (1) {

        /*
         * SERVER -> CLIENT
         */

        ssize_t n = receive_bytes(
            cliente_network,
            temp,
            sizeof(temp)
        );

        if (n > 0) {
            if (framing_append(
                    &frame_buffer,
                    temp,
                    (size_t)n
                ) < 0) {

                printf("Erro de framing\n");
                break;
            }

            while (1) {

                int status = framing_extract(
                    &frame_buffer,
                    message,
                    sizeof(message)
                );

                if (status < 0) {
                    printf("Erro ao extrair mensagem\n");
                    break;
                }

                if (status == 0) {
                    break;
                }

                if (strcmp(message, "/quit\n") == 0) {
                    printf("Servidor encerrou a conexão\n");
                    close_connection(cliente_network);
                    return 0;
                }

                printf("Servidor: %s", message);
            }
        }

        else if (n == 0) {
            printf("Servidor desconectou\n");
            break;
        }

        else {
            printf("Erro no recv\n");
            break;
        }

        /*
         * CLIENT -> SERVER
         */

        if (fgets(
                user_input,
                sizeof(user_input),
                stdin
            ) == NULL) {

            break;
        }

        if (send_all(
                cliente_network,
                user_input,
                strlen(user_input)
            ) < 0) {

            printf("Erro no send\n");
            break;
        }
    }

    close_connection(cliente_network);

    return 0;
}
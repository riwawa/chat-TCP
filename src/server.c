#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>

#include "network.h"
#include "framing.h"
#include "lexer.h"
#include "token_debug.h"

#define SERVER_PORT 9002
#define BACKLOG 5
#define TEMP_BUFFER_SIZE 256
#define MESSAGE_SIZE 256


int start_server(int port)
{
    int server_fd = create_tcp_socket();

    if (server_fd < 0) {
        return -1;
    }

    int opt = 1;

    if (setsockopt(
            server_fd,
            SOL_SOCKET,
            SO_REUSEADDR,
            &opt,
            sizeof(opt)
        ) < 0) {

        close_connection(server_fd);
        return -1;
    }

    struct sockaddr_in endereco_servidor = {0};

    endereco_servidor.sin_family = AF_INET;
    endereco_servidor.sin_port = htons(port);
    endereco_servidor.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(
            server_fd,
            (struct sockaddr *)&endereco_servidor,
            sizeof(endereco_servidor)
        ) < 0) {

        close_connection(server_fd);
        return -1;
    }

    if (listen(server_fd, BACKLOG) < 0) {

        close_connection(server_fd);
        return -1;
    }

    return server_fd;
}


int accept_client(int server_fd)
{
    return accept(
        server_fd,
        NULL,
        NULL
    );
}


int main(void)
{
    int server_fd = start_server(SERVER_PORT);

    if (server_fd < 0) {
        printf("Erro ao iniciar servidor\n");
        return EXIT_FAILURE;
    }

    printf(
        "Servidor escutando na porta %d...\n",
        SERVER_PORT
    );

    /*
     * Por enquanto ainda aceitamos um cliente
     * antes de entrar no poll.
     */
    int client_fd = accept_client(server_fd);

    if (client_fd < 0) {
        printf("Erro ao aceitar cliente\n");

        close_connection(server_fd);

        return EXIT_FAILURE;
    }

    printf("Cliente conectado\n");

    /*
     * Vamos observar o socket do cliente.
     */
    struct pollfd fds[1];

    fds[0].fd = client_fd;
    fds[0].events = POLLIN;

    FrameBuffer frame_buffer = {0};

    char temp[TEMP_BUFFER_SIZE];
    char message[MESSAGE_SIZE];

    while (1) {

        /*
         * Espera até algum FD observado
         * ficar pronto.
         *
         * Como temos apenas um FD,
         * esperamos o client_fd.
         */
        int ready = poll(
            fds,
            1,
            -1
        );

        if (ready < 0) {
            perror("poll");
            break;
        }

        /*
         * O cliente tem dados disponíveis
         * para leitura.
         */
        if (fds[0].revents & POLLIN) {

            ssize_t n = receive_bytes(
                client_fd,
                temp,
                sizeof(temp)
            );

            /*
             * Recebemos bytes.
             */
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

                /*
                 * Um único recv pode trazer
                 * várias mensagens completas.
                 */
                while (1) {

                    int status = framing_extract(
                        &frame_buffer,
                        message,
                        sizeof(message)
                    );

                    /*
                     * Erro de framing.
                     */
                    if (status < 0) {

                        printf(
                            "Erro de framing: "
                            "mensagem grande demais\n"
                        );

                        goto cleanup;
                    }

                    /*
                     * Ainda não temos uma
                     * mensagem completa.
                     */
                    if (status == 0) {
                        break;
                    }

                    /*
                     * Temos uma mensagem completa.
                     * Agora passamos para o lexer.
                     */
                    Lexer lexer;
                    lexer_init(
                        &lexer,
                        message
                    );

                    Token token;

                    do {

                        token = lexer_next(
                            &lexer
                        );

                        print_token(token);

                    } while (
                        token.type != TOK_END &&
                        token.type != TOK_INVALID
                    );

                    printf(
                        "Cliente: %s",
                        message
                    );
                }
            }

            /*
             * recv retornou 0:
             * cliente fechou a conexão.
             */
            else if (n == 0) {

                printf(
                    "Cliente desconectou\n"
                );

                break;
            }

            /*
             * recv retornou erro.
             */
            else {

                printf(
                    "Erro ao receber dados\n"
                );

                break;
            }
        }
    }


cleanup:

    close_connection(client_fd);
    close_connection(server_fd);

    return 0;
}
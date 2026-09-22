#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>

#include "network.h"
#include "framing.h"
#include "lexer.h"
#include "parser.h"
#include "client.h"

#define SERVER_PORT 9002
#define BACKLOG 5

#define TEMP_BUFFER_SIZE 256
#define MESSAGE_SIZE 256

#define MAX_CLIENTS 10


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


void init_clients(Client clients[])
{
    for (int i = 0; i < MAX_CLIENTS; i++) {

        clients[i].fd = -1;
        clients[i].frame_buffer.used = 0;

        clients[i].username[0] = '\0';
        clients[i].authenticated = 0;
    }
}


void init_pollfds(
    struct pollfd fds[],
    int server_fd
)
{
    fds[0].fd = server_fd;
    fds[0].events = POLLIN;
    fds[0].revents = 0;

    for (int i = 0; i < MAX_CLIENTS; i++) {

        int poll_index = i + 1;

        fds[poll_index].fd = -1;
        fds[poll_index].events = POLLIN;
        fds[poll_index].revents = 0;
    }
}


int add_client(
    Client clients[],
    struct pollfd fds[],
    int new_client_fd
)
{
    for (int i = 0; i < MAX_CLIENTS; i++) {

        if (clients[i].fd == -1) {

            clients[i].fd = new_client_fd;
            clients[i].frame_buffer.used = 0;

            clients[i].username[0] = '\0';
            clients[i].authenticated = 0;

            int poll_index = i + 1;

            fds[poll_index].fd = new_client_fd;
            fds[poll_index].events = POLLIN;
            fds[poll_index].revents = 0;

            return i;
        }
    }

    return -1;
}


void remove_client(
    Client clients[],
    struct pollfd fds[],
    int client_index
)
{
    if (clients[client_index].fd != -1) {

        close_connection(
            clients[client_index].fd
        );
    }

    clients[client_index].fd = -1;
    clients[client_index].frame_buffer.used = 0;

    clients[client_index].username[0] = '\0';
    clients[client_index].authenticated = 0;

    int poll_index = client_index + 1;

    fds[poll_index].fd = -1;
    fds[poll_index].events = POLLIN;
    fds[poll_index].revents = 0;
}


void broadcast_message(
    Client clients[],
    int sender_index,
    const char *message
)
{
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].fd == -1) {
            continue;
        }

        if (i == sender_index) {
            continue;
        }

        if (send_all(
                clients[i].fd,
                message,
                strlen(message)
            ) < 0) {

            printf(
                "Erro no broadcast para fd=%d\n",
                clients[i].fd
            );
        }
    }
}


int set_username(
    Client *client,
    const char *username,
    size_t username_length
)
{
    if (username == NULL || username_length == 0) {
        return -1;
    }

    if (username_length >= sizeof(client->username)) {
        return -1;
    }

    memcpy(
        client->username,
        username,
        username_length
    );

    client->username[username_length] = '\0';

    client->authenticated = 1;

    return 0;
}


void process_command(
    Client clients[],
    struct pollfd fds[],
    int client_index,
    Command command,
    const char *original_message
)
{
    Client *client =
        &clients[client_index];

    switch (command.type) {
        case CMD_TEXT:

            if (!client->authenticated) {

                const char *error =
                    "Defina seu username com /name <nome>\n";

                send_all(
                    client->fd,
                    error,
                    strlen(error)
                );

                return;
            }

            printf(
                "%s: %s",
                client->username,
                original_message
            );

            broadcast_message(
                clients,
                client_index,
                original_message
            );

            break;


        /*
         * ----------------------------
         * /name <username>
         * ----------------------------
         */
        case CMD_NAME:

            if (set_username(
                    client,
                    command.argument,
                    command.argument_length
                ) < 0) {

                const char *error =
                    "Username invalido\n";

                send_all(
                    client->fd,
                    error,
                    strlen(error)
                );

                return;
            }

            printf(
                "Cliente fd=%d agora e '%s'\n",
                client->fd,
                client->username
            );

            break;


        /*
         * ----------------------------
         * /quit
         * ----------------------------
         */
        case CMD_QUIT:

            printf(
                "Cliente '%s' solicitou /quit\n",
                client->authenticated
                    ? client->username
                    : "(sem nome)"
            );

            remove_client(
                clients,
                fds,
                client_index
            );

            break;


        /*
         * ----------------------------
         * COMANDO INVALIDO
         * ----------------------------
         */
        case CMD_INVALID:

            {
                const char *error =
                    "Comando invalido\n";

                send_all(
                    client->fd,
                    error,
                    strlen(error)
                );
            }

            break;
    }
}


int handle_client_data(
    Client clients[],
    struct pollfd fds[],
    int client_index
)
{
    Client *client =
        &clients[client_index];

    char temp[TEMP_BUFFER_SIZE];
    char message[MESSAGE_SIZE];

    ssize_t n = receive_bytes(
        client->fd,
        temp,
        sizeof(temp)
    );

    /*
     * =====================================
     * DADOS RECEBIDOS
     * =====================================
     */
    if (n > 0) {

        if (framing_append(
                &client->frame_buffer,
                temp,
                (size_t)n
            ) < 0) {

            printf(
                "Erro de framing no cliente "
                "fd=%d: buffer cheio\n",
                client->fd
            );

            remove_client(
                clients,
                fds,
                client_index
            );

            return -1;
        }

        /*
         * Um único recv() pode conter
         * zero, uma ou várias mensagens.
         */
        while (1) {

            int status = framing_extract(
                &client->frame_buffer,
                message,
                sizeof(message)
            );

            if (status < 0) {

                printf(
                    "Erro de framing no cliente "
                    "fd=%d\n",
                    client->fd
                );

                remove_client(
                    clients,
                    fds,
                    client_index
                );

                return -1;
            }

            /*
             * Ainda não existe frame completo.
             */
            if (status == 0) {
                break;
            }

        
            Lexer lexer;

            lexer_init(
                &lexer,
                message
            );

            Command command =
                parse_command(
                    &lexer
                );

            process_command(
                clients,
                fds,
                client_index,
                command,
                message
            );

            if (clients[client_index].fd == -1) {
                return 0;
            }
        }

        return 0;
    }


    if (n == 0) {

        printf(
            "Cliente fd=%d desconectou\n",
            client->fd
        );

        remove_client(
            clients,
            fds,
            client_index
        );

        return 0;
    }

    printf(
        "Erro ao receber dados "
        "do cliente fd=%d\n",
        client->fd
    );

    remove_client(
        clients,
        fds,
        client_index
    );

    return -1;
}


int main(void)
{
    int server_fd =
        start_server(
            SERVER_PORT
        );

    if (server_fd < 0) {

        printf(
            "Erro ao iniciar servidor\n"
        );

        return EXIT_FAILURE;
    }

    printf(
        "Servidor escutando "
        "na porta %d...\n",
        SERVER_PORT
    );

    Client clients[MAX_CLIENTS];

    struct pollfd fds[
        MAX_CLIENTS + 1
    ];

    init_clients(clients);

    init_pollfds(
        fds,
        server_fd
    );


    while (1) {

        int ready = poll(
            fds,
            MAX_CLIENTS + 1,
            -1
        );

        if (ready < 0) {

            perror("poll");
            break;
        }


        if (fds[0].revents & POLLIN) {

            int new_client_fd =
                accept_client(
                    server_fd
                );

            if (new_client_fd < 0) {

                perror("accept");

            } else {

                int client_index =
                    add_client(
                        clients,
                        fds,
                        new_client_fd
                    );

                if (client_index < 0) {

                    printf(
                        "Servidor cheio. "
                        "Conexão recusada.\n"
                    );

                    close_connection(
                        new_client_fd
                    );

                } else {

                    printf(
                        "Novo cliente conectado: "
                        "fd=%d index=%d\n",
                        new_client_fd,
                        client_index
                    );

                    const char *welcome =
                        "Escolha seu username com "
                        "/name <nome>\n";

                    send_all(
                        new_client_fd,
                        welcome,
                        strlen(welcome)
                    );
                }
            }
        }

        for (
            int i = 0;
            i < MAX_CLIENTS;
            i++
        ) {

            if (clients[i].fd == -1) {
                continue;
            }

            int poll_index =
                i + 1;

            if (!(
                fds[poll_index].revents
                & POLLIN
            )) {
                continue;
            }

            handle_client_data(
                clients,
                fds,
                i
            );
        }
    }

    for (
        int i = 0;
        i < MAX_CLIENTS;
        i++
    ) {

        if (clients[i].fd != -1) {

            close_connection(
                clients[i].fd
            );
        }
    }

    close_connection(
        server_fd
    );

    return 0;
}

#include <stdio.h>

#include <stdlib.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <unistd.h> 
#include <netinet/in.h> 

int main(){
    int servidor_network;
    servidor_network = socket(AF_INET, SOCK_STREAM, 0);

    if (servidor_network < 0) {
        printf("Erro ao criar o socket (Código: %d)\n", servidor_network);
        exit(EXIT_FAILURE);
    }
    int opt = 1;
    setsockopt(servidor_network, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in endereco_servidor;
    endereco_servidor.sin_family = AF_INET;
    endereco_servidor.sin_port = htons(9002);
    endereco_servidor.sin_addr.s_addr = INADDR_ANY;

    if (bind(servidor_network, (struct sockaddr*) &endereco_servidor, sizeof(endereco_servidor)) < 0){
    printf("Binding the socket failed!\n\n");
    exit(EXIT_FAILURE);
    }

    if (listen(servidor_network, 5) < 0){
    printf("Socket listening failed\n\n");
    exit(EXIT_FAILURE);
    }

    int client_socket = accept(servidor_network,NULL,NULL);
    if (client_socket < 0){
    printf("Client socket is negative, couldn't accept");
    exit(EXIT_FAILURE);
    }

    char server_message[256] = "Olá, mundo!"; 
    send(client_socket, server_message, sizeof(server_message), 0);

    close(servidor_network);
    close(client_socket);


    return 0;
}
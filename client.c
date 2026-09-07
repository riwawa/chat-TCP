#include <stdio.h>
#include <stdlib.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <unistd.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <string.h>

int main(){
    int cliente_network;
    cliente_network = socket(AF_INET, SOCK_STREAM, 0);

    if (cliente_network < 0) {
        printf("Erro ao criar o socket (Código: %d)\n", cliente_network);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in endereco_servidor;
    endereco_servidor.sin_family = AF_INET;
    endereco_servidor.sin_port = htons(9002);
    endereco_servidor.sin_addr.s_addr = inet_addr("127.0.0.1");

    int connection_status = connect(cliente_network, (struct sockaddr*) &endereco_servidor, sizeof(endereco_servidor));

    if (connection_status < 0) {
        printf("Erro ao conectar ao servidor (%d)\n", connection_status);
        exit(EXIT_FAILURE);
    }
    
    
    char server_response[256];
    char buffer[256];

    while(1){
        
        ssize_t n = recv(cliente_network, buffer, 255, 0);
        
        if (n>0){
            buffer[n] = '\0';
            if (strcmp(buffer, "/quit\n")==0) {
                break;
            } else {
                printf("Servidor: %s", buffer);
            }
        } else if (n==0){
            printf("Servidor desconectou\n");
            break;
        } else {
            printf("Erro no recv\n");
            break;
        }

        if (fgets(server_response, sizeof(server_response), stdin) == NULL) {
            break;
        }

        ssize_t enviados = send(cliente_network, server_response, strlen(server_response), 0);
        if (enviados < 0) {
            printf("Erro no send\n");
            break;
        }
    }

    close(cliente_network);

    return 0;
}

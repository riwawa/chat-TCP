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
    
    
    char user_input[256];
    char temp[256];
    char recv_buffer[1024];
    size_t recv_used = 0;
    while(1){
        
        ssize_t n = recv(cliente_network, temp, sizeof(temp), 0);

            
        if (n>0){

            if (recv_used + n > sizeof(recv_buffer)) {
                printf("Buffer cheio\n");
                break;
            }

            memcpy(recv_buffer + recv_used, temp, n); 
            recv_used += n;

            char *newline;
            while ((newline=memchr(recv_buffer,'\n', recv_used)) != NULL) {
                size_t message_len = newline - recv_buffer + 1;
                char message[256];
                if (message_len >= sizeof(message)) {
                    printf("Mensagem grande demais\n");
                    break;
                }

                memcpy(message, recv_buffer, message_len);
                message[message_len] = '\0';

                if (strcmp(message, "/quit\n") == 0){
                    printf("Servidor encerrou a conexão\n");
                    close(cliente_network);
                    return 0;
                }

                printf("Servidor: %s", message);

                size_t remaining = recv_used - message_len;

                memmove(recv_buffer, recv_buffer + message_len, remaining);
                recv_used = remaining;
            }
        } else if (n==0){
            printf("Servidor desconectou\n");
            break;
        } else {
            printf("Erro no recv\n");
            break;
        }

        if (fgets(user_input, sizeof(user_input), stdin) == NULL) {
            break;
        }

        ssize_t enviados = send(cliente_network, user_input, strlen(user_input), 0);
        if (enviados < 0) {
            printf("Erro no send\n");
            break;
        }
    }

    close(cliente_network);

    return 0;
}

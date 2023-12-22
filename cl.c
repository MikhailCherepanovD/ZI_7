#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>


#define SERVER_PORT 8080 
//#define MSG_MAX_LENGTH 80 
#define SA struct sockaddr

int main () {
    int client_socket;
    struct sockaddr_in server_address;


    client_socket = socket(AF_INET,SOCK_STREAM, 0);
    if(client_socket < 0){
        perror("Error creating socket");
        return 2;
    }
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(SERVER_PORT);



    if (connect(client_socket, (SA*)&server_address, sizeof(server_address)) < 0){
        perror("Error connecting ");
        return 2;
    }

    int id = fork();

    if (id < 0){
        perror("Creating process finished with error");
        return 2;
    }

    if(id > 0){

        while(1){
            char message[256];
            printf("$");
            fgets(message, sizeof(message), stdin);

            if(strcmp(message,"exit\n") == 0){
                kill(id, SIGTERM);
                break;
            }
            // Sending message
            
            if(send(client_socket, message, strlen(message),0) < 0){
                perror("Error sending message");
                return 2;
            }
        }   
        // Kill process
        shutdown(client_socket,SHUT_RDWR);
        close(client_socket);
        exit(0);
    } 
    else {
        // Id process recieve message and show it
        while(1){
            char recv_message[256];
            memset(recv_message,0,sizeof(recv_message));

            if(recv(client_socket,recv_message,sizeof(recv_message),0) < 0){
                perror("Error recieve message");
                return 2;
            }

            if(strlen(recv_message) == 0){
                printf("Server disconnect\n");
                break;
            }
            printf("\n$Server: %s",recv_message);

        }
       
        close(client_socket);
        exit(0);
    }

    printf("Exit\n");
    return 0;
}




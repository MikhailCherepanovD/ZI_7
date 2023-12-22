#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>


#define SERVER_PORT 8080 
#define MSG_MAX_LENGTH 80 
#define SA struct sockaddr

int main () {
    // Creating socket

    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1, address_length =  sizeof(address);

    char buffer[MSG_MAX_LENGTH] = {0};

    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        perror("Error create socket");
        return 2;
    }

    // Set params socket

    if (setsockopt(server_fd, SOL_SOCKET,SO_REUSEADDR, &opt, sizeof(opt))){
        perror("Error set params");
        return 2;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(SERVER_PORT);

    // Connect socket
    if(bind(server_fd,(SA*)&address,sizeof(address)) < 0){
        perror("Error binding");
        return 2;
    }

    if(listen(server_fd, 3) < 0){
        perror("Error with listening");
        return 2;
    }

    if((new_socket = accept(server_fd, (SA*)&address, (socklen_t*)&address_length)) < 0){
        perror("Error with accepting another connections");
        return 2;
    }

    int id = fork();

    if (id < 0){
        perror("Creating process finished with error");
        return 2;
    }

    if(id > 0){

        while(1){
            printf("$");
            fgets(buffer, MSG_MAX_LENGTH, stdin);
            // Sending message
            send(new_socket, buffer, strlen(buffer),0);
        }
        // Kill process
        kill(id,SIGTERM);
        shutdown(new_socket,SHUT_RDWR);
        close(new_socket);
        exit(0);
    } 
    else {
        // Id process recieve message and show it
        while(1){
            valread = recv(new_socket, buffer, MSG_MAX_LENGTH, 0);

            if(valread == 0){
                // If of message length == 0
                printf("Connection closed.\n");
                break;
            }

            printf("\n$Client: %s", buffer); 
        }
        shutdown(new_socket,SHUT_RDWR);
        close(new_socket);
        exit(0);
    }

    printf("Good bye!\n");
    return 0;
}




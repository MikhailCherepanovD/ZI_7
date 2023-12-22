#include <openssl/blowfish.h>
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define SERVER_PORT 8080
#define MAX 80
#define SA struct sockaddr

const char *key = "qaz123wsx456edc789";
const char *stop_word = "exit";
BF_KEY bfkey;
unsigned char ivec[8];

void cryptographic(const unsigned char *in, unsigned char *out, long len, int etc){
    BF_set_key(&bfkey, strlen(key), key);
    bzero(ivec, sizeof(ivec));
    BF_cbc_encrypt(in, out, len, &bfkey, ivec, etc);
}


char active(int PID){
    /* 
        Checking if PID is active 
        input: PID (int)
        output: 1,0 (char)
    */
    return (waitpid(PID, NULL, WNOHANG) > 0) ? 0 : 1;
}

void read_msg(int sockfd, int writePID)
{
    char buffer[MAX];   
    char buffer_crypto[MAX];
    char *working_buffer = buffer;
    while(1)
    {
        // Check if server doesnt here
        if (!active(writePID))
        {
            printf("\nExited successfully.\n");
            break;
        }
        bzero(buffer, MAX);
        
        read(sockfd, buffer, sizeof(buffer));
        if (!active(writePID))
        {
            printf("\nExited successfully.\n");
            break;
        }
        
        bzero(buffer_crypto, MAX);
        cryptographic(buffer, buffer_crypto, MAX, BF_DECRYPT);
        
        char correct = 1;

        if (working_buffer[MAX - 1] != 0 && working_buffer[0] != 0)
        {
            correct = 0;

            bzero(buffer_crypto, MAX);
            cryptographic(buffer, buffer_crypto, MAX, BF_DECRYPT);
            if (buffer_crypto[MAX - 1] == 0)
            {
        
                working_buffer = buffer_crypto;
                correct = 1;
            }
        }
        if (correct)
        {
            if (strncmp(stop_word, working_buffer, strlen(stop_word)) == 0)
            {
                printf("Server disconnect\n");
                kill(writePID, SIGKILL);
                break;
            }

            if (working_buffer[0] != 0)
            {
                printf("\n$: %s", working_buffer);
            }
        }
        else
        {
            printf("\nIncoming message is not valid.\n");
        }
    }
}

void write_msg(int sockfd)
{
    char buffer[MAX];
    char buffer_crypto[MAX];
    char *working_buffer = buffer;
    int i = 0;
    working_buffer = buffer_crypto;
    printf("\n$ ");
    while(1)
    {
        bzero(buffer, MAX);
        i = 0;
        while ((buffer[i++] = getchar()) != '\n')
            if (i == MAX - 2)
            {
                buffer[i] = '\n';
                buffer[i + 1] = '\0';
                break;
            }

        bzero(buffer_crypto, MAX);
        cryptographic(buffer, buffer_crypto, MAX, BF_ENCRYPT);
        write(sockfd, working_buffer, sizeof(buffer));
        if (strncmp(stop_word, buffer, strlen(stop_word)) == 0)
        {
            printf("Server disconnect\n");
            exit(0);
        }
    }
}

int main(int argc, char *argv[])
{
    int sockfd, connfd, i;
    struct sockaddr_in address, cli;
    const char *localhost = "127.0.0.1";
    int err = 0;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("Failed to create a socket.\n");
        exit(0);
    }

    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(SERVER_PORT);

    if (connect(sockfd, (SA *)&address, sizeof(address)) < 0)    {
        printf("Failed to connect to the server.\n");
        exit(0);
    }

    int id = fork();
    if (id < 0){
        printf("Forking error :(.\n");
        exit(0);
    }
    
    if (id == 0){
        write_msg(sockfd);
        exit(0);
    }
    else
    {
        read_msg(sockfd, id);
    }

    close(sockfd);
    return 0;
}

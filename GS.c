#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>

#define TRUE 1
#define FALSE 0
#define DEFAULT_GSport "58033" 
#define PORT_SIZE 16

int verbose = FALSE;
int udp_fd, errcode;
ssize_t n;
socklen_t addrlen;
struct addrinfo hints, *res;
struct sockaddr_in addr;
char buffer[128];

int main(int argc, char *argv[]){
    char GSport[PORT_SIZE];
    strcpy(GSport, DEFAULT_GSport);

    for(int i = 0; i < argc - 1; i++){
        if(strcmp(argv[i], "-p") == 0){
            strcpy(GSport, argv[i+1]);
        }
        if(strcmp(argv[i], "-v") == 0){
            verbose = TRUE;
        }
    }

    while(1){
    }
}
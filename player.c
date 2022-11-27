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

#define DEFAULT_GSIP ""
#define DEFAULT_GSport "" 
#define PORT_SIZE 16
#define IP_SIZE 64

int tcp_fd, udp_fd, errcode;
ssize_t n;
socklen_t addrlen;
struct addrinfo udp_hints, tcp_hints, *udp_res, *tcp_res;
struct sockaddr_in addr;
char buffer[128];

char start(char plid);
char play(char letter);
void received_udp(char *received);
void received_tcp(char *received);

int main(int argc, char *argv[]){
    //arg tem default com IPaddress e PORT
    
    char GSport[PORT_SIZE], GSIP[IP_SIZE];
    strcpy(GSport, DEFAULT_GSport);
    strcpy(GSIP, DEFAULT_GSIP);

    for(int i = 0; i < argc - 1; i++){
        if(strcmp(argv[i], "-p") == 0){
            strcpy(GSport, argv[i+1]);
        }
        if(strcmp(argv[i], "-n") == 0){
            strcpy(GSIP, argv[i+1]);
        }
    }

    char input[30], *command;
    char send[30];
    
    //udp
    udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(udp_fd == -1) exit(1);
    memset(&udp_hints, 0, sizeof udp_hints);

    udp_hints.ai_family = AF_INET;
    udp_hints.ai_socktype = SOCK_DGRAM;

    errcode = getaddrinfo("tejo.tecnico.ulisboa.pt", GSport, &udp_hints, &udp_res);
    if(errcode != 0) exit(1);
    
    //tcp
    tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(tcp_fd == -1) exit(1);
    memset(&tcp_hints, 0, sizeof tcp_hints);

    tcp_hints.ai_family = AF_INET;
    tcp_hints.ai_socktype = SOCK_STREAM;

    errcode = getaddrinfo("tejo.tecnico.ulisboa.pt", GSport, &tcp_hints, &tcp_res);
    if(errcode != 0) exit(1);
    char aux;

    for(;;){
        fgets(input, 30 + 1, stdin);
        command = strtok(input, " \t\n");
        int t_u = -1;
        
        if(strcmp(command, "start") == 0|| strcmp(command, "sg") == 0){
            aux = start(*command);
            strcpy(send, &aux);
            t_u = 1;
        } 
        else if(strcmp(command, "play") == 0|| strcmp(command, "pl") == 0){
            //aux = play(*input);
            //strcpy(send, play(*input));
            t_u = 1;
        }
        else if(strcmp(command, "guess")== 0 || strcmp(command, "gw") == 0){
            t_u = 1;
        }
        else if(strcmp(command, "scorebord") == 0 || strcmp(command, "sb") == 0){
            t_u = 0;
        }
        else if(strcmp(command, "hint") == 0 || strcmp(command, "h") == 0){
            t_u = 0;
        }
        else if(strcmp(command, "state") == 0 || strcmp(command, "st") == 0){
            t_u = 0;
        }
        else if(strcmp(command, "quit") == 0){
            t_u = 1;
        }
        else if(strcmp(command, "exit") == 0){
            t_u = 1;
        }

        if(t_u == 0){
            n = connect(tcp_fd, tcp_res->ai_addr, tcp_res->ai_addrlen);
            if(n == -1) exit(1);

            n = write(tcp_fd, send, strlen(send));
            if(n == -1) exit(1);

            n = read(tcp_fd, buffer, 128);
            if(n == -1) exit(1);
            
            received_tcp(buffer);
        }
        
        if(t_u == 1){
            n = sendto(udp_fd, send, strlen(send), 0, udp_res->ai_addr, udp_res->ai_addrlen);
            if(n == -1) exit(1);

            addrlen = sizeof(addr);
            n = recvfrom(udp_fd, buffer, 128, 0, (struct sockaddr*) &addr, &addrlen);
            if(n == -1) exit(1);

            received_udp(buffer);
        }
        memset(buffer,0,strlen(buffer));
        memset(send,0,strlen(send));
        memset(input,0,strlen(input));
    }

    return 0;
}

char start(char plid){
    char out[30];
    strcpy(out,  "SNG");
    strcat(out, &plid);
    return *out;
}

void received_udp(char *received){
    char *command = strtok(received, " \t\n");
    if(strcmp(command, "RSG") == 0){
        // que acontece quando status é NOK? não tendo n_letters nem max_errors, será que envia a palavra do jogo não finaalizado?
    }
    //função que trata dos prints visiveis para o player
}

void received_tcp(char *received){
    
}

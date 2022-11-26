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
#define PORT "58001"

int tcp_fd, udp_fd, errcode;
ssize_t n;
socklen_t addrlen;
struct addrinfo udp_hints, tcp_hints, *udp_res, *tcp_res;
struct sockaddr_in addr;
char buffer[128];

char *start(char plid);
char *play(char letter);
void received_udp(char received);
void received_tcp(char received);


int main(char args){
    //arg tem default com IPaddress e PORT

    char input[30], *command;
    char send[30];
    
    //udp
    udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(udp_fd == -1) exit(1);
    memset(&udp_hints, 0, sizeof udp_hints);

    udp_hints.ai_family = AF_INET;
    udp_hints.ai_socktype = SOCK_DGRAM;

    errcode = getaddrinfo("tejo.tecnico.ulisboa.pt", PORT, &udp_hints, &udp_res);
    if(errcode != 0) exit(1);
    
    //tcp
    tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(tcp_fd == -1) exit(1);
    memset(&tcp_hints, 0, sizeof tcp_hints);

    tcp_hints.ai_family = AF_INET;
    tcp_hints.ai_socktype = SOCK_STREAM;

    errcode = getaddrinfo("tejo.tecnico.ulisboa.pt", PORT, &tcp_hints, &tcp_res);
    if(errcode != 0) exit(1);
    
    for(;;){
        fgets(input, 30 + 1, stdin);
        command = strtok(input, " \t\n");
        int t_u = -1;
        
        if(strcmp(command, "start") == 0|| strcmp(command, "sg") == 0){
            send = start(input + strlen(command) + 1);
            t_u = 1;
        } 
        else if(strcmp(command, "play") == 0|| strcmp(command, "pl") == 0){
            send = play(input);
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
        
    }

    return 0;
}

char *start(char plid){
    char *out;
    out = strcat("SNG", plid);
    return out;
}

void received_udp(char received){
    char *command = strtok(received, " \t\n");
    if(strcmp(command, "RSG") == 0){
        // que acontece quando status é NOK? não tendo n_letters nem max_errors, será que envia a palavra do jogo não finaalizado?
    }
    //função que trata dos prints visiveis para o player
}

void received_tcp(char received){
    
}

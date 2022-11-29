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

#define WHITESPACE " "
#define DEFAULT_GSIP "tejo.tecnico.ulisboa.pt"
#define DEFAULT_GSport "58033"
#define INPUT_SIZE 30
#define PORT_SIZE 16
#define IP_SIZE 64
#define PLID_SIZE 7

int tcp_fd, udp_fd, errcode;
char id[PLID_SIZE];
ssize_t n;
socklen_t addrlen;
struct addrinfo udp_hints, tcp_hints, *udp_res, *tcp_res;
struct sockaddr_in addr;
char buffer[128];
int n_trials = 0;

char start(char plid[]);
char play(char letter);
char guess(char word);
void communication_upd(char *send);
void communication_tcp(char *send);
void received_udp(char *received);
void received_tcp(char *received);

int main(int argc, char *argv[]){
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

    char input[INPUT_SIZE], *command;
    char send[INPUT_SIZE];
    
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
        fgets(input, INPUT_SIZE, stdin);

		char *token_list[7];
		char *tok = strtok(input, " \n");
		for (int i = 0; tok != NULL && i < 7; i++){
			token_list[i] = tok;
			tok = strtok(NULL, " \n");
		}
        int t_u = -1;
        
        if(strcmp(token_list[0], "start") == 0 || strcmp(token_list[0], "sg") == 0){
            aux = start(token_list[1]);
            strcpy(send, &aux);
            t_u = 1;
        } 
        else if(strcmp(token_list[0], "play") == 0 || strcmp(token_list[0], "pl") == 0){
            aux = play(*token_list[1]);
            strcpy(send, &aux);
            n_trials++;
            t_u = 1;
        }
        else if(strcmp(token_list[0], "guess")== 0 || strcmp(token_list[0], "gw") == 0){
            aux = guess(*token_list[1]);
            n_trials++;
            t_u = 1;
        }
        else if(strcmp(token_list[0], "scorebord") == 0 || strcmp(token_list[0], "sb") == 0){
            t_u = 0;
        }
        else if(strcmp(token_list[0], "hint") == 0 || strcmp(token_list[0], "h") == 0){
            t_u = 0;
        }
        else if(strcmp(token_list[0], "state") == 0 || strcmp(token_list[0], "st") == 0){
            t_u = 0;
        }
        else if(strcmp(token_list[0], "quit") == 0){
            t_u = 1;
        }
        else if(strcmp(token_list[0], "exit") == 0){
            t_u = 1;
        }

        if(t_u == 0){
            communication_tcp(send);
        }
        
        if(t_u == 1){
            communication_upd(send);
        }

        memset(buffer, 0, strlen(buffer));
        memset(send, 0, strlen(send));
        memset(input, 0, strlen(input));
        memset(&aux, *WHITESPACE, strlen(&aux));
    }

    return 0;
}

char start(char plid[]){
    char out[50];
    strcpy(out,  "SNG ");
    strcat(out, plid);
    strcpy(id, plid);
    return *out;
}

char play(char letter){
    char out[INPUT_SIZE];
    char str[5];
    strcpy(out,  "PLG ");
    strcat(out, id);
    strcat(out, WHITESPACE);
    strcat(out, &letter);
    sprintf(str, " %d", n_trials);
    strcat(out, str);
    printf("%s", out);
    return *out;
}

char guess(char word){
    char out[INPUT_SIZE];
    char str[5];
    strcpy(out,  "PLG ");
    strcat(out, id);
    strcat(out, WHITESPACE);
    strcat(out, &word);
    sprintf(str, " %d", n_trials);
    strcat(out, str);
    return *out;
}

void communication_upd(char *send){
    n = sendto(udp_fd, send, strlen(send), 0, udp_res->ai_addr, udp_res->ai_addrlen);
    if(n == -1) exit(1);

    addrlen = sizeof(addr);
    n = recvfrom(udp_fd, buffer, 128, 0, (struct sockaddr*) &addr, &addrlen);
    if(n == -1) exit(1);

    received_udp(buffer);
}

void communication_tcp(char *send){
    n = connect(tcp_fd, tcp_res->ai_addr, tcp_res->ai_addrlen);
    if(n == -1) exit(1);

    n = write(tcp_fd, send, strlen(send));
    if(n == -1) exit(1);

    n = read(tcp_fd, buffer, 128);
    if(n == -1) exit(1);
    
    received_tcp(buffer);
}

void received_udp(char *received){
    char *command = strtok(received, " \n");
    if(strcmp(command, "RSG") == 0){
        // que acontece quando status é NOK? não tendo n_letters nem max_errors, será que envia a palavra do jogo não finaalizado?
    }
    //função que trata dos prints visiveis para o player
}

void received_tcp(char *received){}

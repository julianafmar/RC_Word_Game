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
char word_file[30];
char word[30];
char word_hint[30];
char buffer[128];

int main(int argc, char *argv[]){
    int line_number;
    char command[3];
    char plid[6];
    char GSport[PORT_SIZE];
    strcpy(GSport, DEFAULT_GSport);
    strcpy(word_file, argv[1]);

    for(int i = 0; i < argc - 1; i++){
        if(strcmp(argv[i], "-p") == 0){
            strcpy(GSport, argv[i+1]);
        }
        if(strcmp(argv[i], "-v") == 0){
            verbose = TRUE;
        }
    }

    while(1){
        //fzr coisinhas do tcp e udp

        if(n != 0){
            char *ptr = buffer;
            sscanf(ptr, "%s", command);
            ptr += strlen(command);
            if(strcmp(command, "SNG") == 0){
                sscanf(ptr, "%s", plid);

                if("idk"){
                    "status thing NOK";
                }
                
                else if("idk"){
                    FILE *fp = fopen(word_file, "r");
                    if(fp == NULL) exit(1);

                    char line[50];
                    for(int i = 0; fgets(line, sizeof(line), fp) != NULL && i <= 30; i++){
                        if(i == line_number){
                            sscanf(line, "%s %s", word, word_hint);
                            break;
                        }
                    }
                    fclose(fp);
                }
                
            }
        }
    }
}
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
#define MAX_LINES 25

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

void start(char plid[], int line_number);
void play(char plid[], char letter, int trial);

int main(int argc, char *argv[]){
    int n_trials = 0;
    int line_number = 0;
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
            char send[80];
            sscanf(ptr, "%s", command);
            ptr += strlen(command);
            if(strcmp(command, "SNG") == 0){
                sscanf(ptr, "%s", plid);
                start(plid, line_number);
            }
            if(strcmp(command, "PLG") == 0){
                char letter;
                int trial;
                sscanf(ptr, "%s %s %d", plid, letter, trial);
                play(plid, letter, trial);
            }
        }
    }
}

void start(char plid[], int line_number){
    char game_file[30];
    int max_errors;

    sprintf(game_file, "GAME/GAME_%s.txt", plid);
    if(access(game_file, F_OK) == 0){
        sprintf(send, "RSG NOK\n");
        //return NOK to client;
        //maybe access is not enough, i think we need to see if we have plays inside the file
    }
    else{
        FILE *fp = fopen(word_file, "r");
        if(fp == NULL) exit(1);

        char line[50];
        for(int i = 0; fgets(line, sizeof(line), fp) != NULL && i <= MAX_LINES; i++){
            if((i % MAX_LINES) == line_number){
                sscanf(line, "%s %s", word, word_hint);
                break;
            }
        }
        line_number++;
        fclose(fp);

        int word_len = strlen(word);
        if(word_len <= 6){
            max_errors = 7;
        }
        else if(word_len > 7 && word_len <= 10){
            max_errors = 8;
        }
        else if(word_len >= 11){
            max_errors = 9;
        }

        FILE *fp = fopen(game_file, "w");
        fwrite(line, 1, strlen(line), fp);
        fclose(fp);

        sprintf(send, "RSG OK %ld %s\n", word_len, max_errors);
        //send to player
    }
}

void play(char plid[], char letter, int trial){
    
}
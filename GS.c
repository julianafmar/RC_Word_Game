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
#include <dirent.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#define TRUE 1
#define FALSE 0
#define DEFAULT_GSport "58033" 
#define PORT_SIZE 16
#define MAX_LINES 25
#define FILE_SIZE 30
#define WORD_SIZE 100

int verbose = FALSE;
int udp_fd, tcp_fd, errcode;
ssize_t n;
socklen_t addrlen;
struct addrinfo hints, *res;
struct sockaddr_in addr;

int start(char plid[], int line_number, char word_file[]);
int play(char plid[], char letter, int trial);
void scoreboard();
int getMaxErrors(int word_len);
void fileWrite(char file_name[], char write[], char type[]);
void changeGameDir(char filename[], char plid[], char code);
void scoreCreate();

int main(int argc, char *argv[]){
    pid_t pid;
    int n_trials = 0;
    int line_number = 0;
    char command[3];
    char plid[6];
    char word_file[FILE_SIZE];
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

    //UDP
    udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(udp_fd == -1) exit(1);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    errcode = getaddrinfo(NULL, GSport, &hints, &res);
    if(errcode != 0) exit(1);
    n = bind(udp_fd, res->ai_addr, res->ai_addrlen);
    if(n == -1) exit(1);

    //TCP
    tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(tcp_fd == -1) exit(1);
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    errcode = getaddrinfo(NULL, GSport, &hints, &res);
    if((errcode) != 0) exit(1);
    n = bind(tcp_fd, res->ai_addr, res->ai_addrlen);
    if(n == -1) exit(1);
    if(listen(tcp_fd, 5) == -1) exit(1);

    while(1){
        char buffer[128];
        if((pid = fork()) == 0){
            addrlen = sizeof(addr);
            int newfd = accept(tcp_fd, (struct sockaddr*) &addr, &addrlen);
            if(newfd == -1) exit(1);
            n = read(newfd, buffer, 128);
            if(n == -1) exit(1);
        }
        else{
            addrlen = sizeof(addr);
            n = recvfrom(udp_fd, buffer, 128, 0, (struct sockaddr*) &addr, &addrlen);
            if(n == -1) exit(1);
        }

        char *ptr = buffer;
        sscanf(ptr, "%s", command);
        ptr += strlen(command);

        if(strcmp(command, "SNG") == 0){
            sscanf(ptr, "%s", plid);
            start(plid, line_number, word_file);
        }
        else if(strcmp(command, "PLG") == 0){
            char letter, c_trial[2];
            sscanf(ptr, "%s %c %s", plid, &letter, c_trial);
            int trial = atoi(c_trial);
            play(plid, letter, trial);
        }
        else if(strcmp(command, "GSB") == 0){
            scoreboard();
        }
    }
}

void udpSendToClient(char buffer[]){
    n = sendto(udp_fd, buffer, n, 0, (struct sockaddr*) &addr, addrlen);
    if(n == -1) exit(1);
}

void tcpSendToClient(char buffer[]){
    n = write(tcp_fd, buffer, n);
    if(n == -1) exit(1);
}

int start(char plid[], int line_number, char word_file[]){
    char game_file[FILE_SIZE];
    int word_len;
    char word[30];
    char send[80];
    char line[50];
    int max_errors;

    sprintf(game_file, "GAME/GAME_%s.txt", plid);
    if(access(game_file, F_OK) == 0){
        FILE *fp = fopen(game_file, "r");
        if(fp == NULL) exit(1);

        int i = 0;
        for(; fgets(line, sizeof(line), fp) != NULL && i <= MAX_LINES; i++){
            if(i == 0){
                sscanf(line, "%s", word);
            }
        }
        if(i == 0){
            word_len = strlen(word);
            max_errors = getMaxErrors(word_len);
            sprintf(send, "RSG OK %d %d\n", word_len, max_errors);
            udpSendToClient(send);
        }
        else{
            strcpy(send, "RSG NOK\n");
            udpSendToClient(send);
        }
        fclose(fp);
        return 0;
    }
    else{
        FILE *fp = fopen(word_file, "r");
        if(fp == NULL) exit(1);
        
        for(int i = 0; fgets(line, sizeof(line), fp) != NULL && i <= MAX_LINES; i++){
            if((i % MAX_LINES) == line_number){
                sscanf(line, "%s", word);
                break;
            }
        }
        line_number++;
        fclose(fp);

        word_len = strlen(word);
        max_errors = getMaxErrors(word_len);

        FILE *fp_game = fopen(game_file, "w");
        fwrite(line, 1, strlen(line), fp);
        fclose(fp_game);

        sprintf(send, "RSG OK %d %d\n", word_len, max_errors);
        udpSendToClient(send);
        return 0;
    }
}

int play(char plid[], char letter, int trial){
    char game_file[FILE_SIZE];
    char send[80];
    char code[2];
    char play_wl[WORD_SIZE];
    int right_trials = 0;
    int wrong_trials = 0;
    char word[WORD_SIZE];

    sprintf(game_file, "GAME_%s.txt", plid);
    
    if(access(game_file, F_OK) != 0){
        udpSendToClient("RLG ERR\n");
        return 0;
    }
    
    FILE *fp = fopen(game_file, "rw");
    if(fp == NULL) exit(1);
    
    char line[50];
    fgets(line, sizeof(line), fp);
    sscanf(line, "%s", word);
    int i = 1;
    for(; fgets(line, sizeof(line), fp) != NULL; i++){
        sscanf(line, "%s %s", code, play_wl);
        if(strlen(play_wl) == 1){
            char *let = play_wl;
            if(letter == *let){
                sprintf(send, "RLG DUP %d\n", trial); //é suposto ter sempre o trial a seguir?
                if(trial != i){
                    udpSendToClient(send);
                    fclose(fp);
                    return 0;
                }
            }
        }
        
        if(strcmp("T", code) == 0){
            if(strstr(word, play_wl) != NULL) right_trials++;
            else wrong_trials++;
        }
    }
    if(i != trial){
        udpSendToClient("RLG INV\n");
        fclose(fp);
        return 0;
    }
    else if(strchr(word, letter) != NULL){
        int new_pos = 0;
        char positions[30];
        sprintf(positions, "%c", letter);
        for(int i = 0; i < strlen(word); i++){
            if(word[i] == letter){
                char aux[3];
                sprintf(aux, " %d", (i+1));
                strcat(positions, aux);
                new_pos++;
            }
        }
        strcat(positions, "\n");
        if((right_trials + new_pos) == strlen(word)){
            char write[WORD_SIZE];
            sprintf(write, "T %c\n", letter);
            fileWrite(game_file, write, "a");
            changeGameDir(game_file, plid, 'W');
            udpSendToClient("RLG WIN\n");
            fclose(fp);
            return 0;
        }
        else{
            char write[WORD_SIZE];
            sprintf(write, "T %c\n", letter);
            fileWrite(game_file, write, "a");
            udpSendToClient("RLG OK\n");
            fclose(fp);
            return 0;
        }
    }
    else if(getMaxErrors(strlen(word)) == wrong_trials++){
        char write[WORD_SIZE];
        sprintf(write, "T %c\n", letter);
        fileWrite(game_file, write, "a");
        changeGameDir(game_file, plid, 'F');
        udpSendToClient("RLG OVR\n");
        fclose(fp);
        return 0;
    }
    else if(getMaxErrors(strlen(word)) > wrong_trials++){
        char write[WORD_SIZE];
        sprintf(write, "T %c\n", letter);
        fileWrite(game_file, write, "a");
        udpSendToClient("RLG NOK\n");
        fclose(fp);
        return 0;
    }

    fclose(fp);
    return 0;
}

void scoreboard(){

}

int getMaxErrors(int word_len){
    if(word_len <= 6){
        return 7;
    }
    else if(word_len > 7 && word_len <= 10){
        return 8;
    }
    else if(word_len >= 11){
        return 9;
    }
}

void fileWrite(char file_name[], char write[], char type[]){
    FILE *fp = fopen(file_name, type);
    fwrite(write, 1, strlen(write), fp);
    fclose(fp);
}

void changeGameDir(char filename[], char plid[], char code){
    char path[15];
    char new_filename[30];
    sprintf(path, "GAMES/%s", plid);

    DIR* dir = opendir(path);
    if(ENOENT == errno){
        mkdir(path, 0777);
    }

    struct stat attr;
    struct tm *t;
    stat(filename, &attr);
    t = gmtime(&attr.st_mtime);

    sprintf(new_filename, "GAMES/%s/%d%d%d_%d%d%d_%c", plid, t->tm_year, t->tm_mon, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, code);
    int m = rename(filename, new_filename);
    if(code == 'W') scoreCreate();
}

void scoreCreate(){

}
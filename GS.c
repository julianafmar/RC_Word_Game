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
#define FNAME_SIZE 24
#define SEND_SIZE 129
#define COMMAND_SIZE 4
#define PLID_SIZE 7

int verbose = FALSE;
int udp_fd, tcp_fd, newfd, errcode;
ssize_t n;
socklen_t addrlen;
struct addrinfo udp_hints, tcp_hints, *udp_res, *tcp_res;
struct sockaddr_in addr;

void verbosePrint(char plid[], char command[]);
void start(char plid[], char word_file[], char buffer[], int line_number);
void play(char plid[], char letter, int trial);
void guess(char plid[], char guess_word[], int trial);
void quit(char plid[]);
void rev(char plid[]);
void scoreboard(int pid);
void hint(char plid[]);
void state(char plid[]);
int getMaxErrors(int word_len);
void fileWrite(char file_name[], char write[], char type[]);
void changeGameDir(char filename[], char plid[], char code);
void scoreCreate(int n_succ, int n_wrong, char plid[], char filename[], char word[]);
void udpSendToClient(char buffer[], int c);
void tcpSendToClient(char buffer[], int c);
void tcpSendFile(FILE *fp);
int FindLastGame(char plid[], char *fname);

//signal handler ----------------
#include <signal.h>
void signalHandler(int sig){
    signal(sig, SIG_IGN);

    /*struct dirent **player_score;
    int m =  scandir(".", &player_score, NULL, alphasort);
    if(m < 0) perror("scandir");
    for(int i = 0; i < m; i++){
        if(strstr(player_score[i]->d_name, "GAME_")){
            char plid[PLID_SIZE];
            sscanf(player_score[i]->d_name, "GAME_%s.txt", plid);
            plid[6] = '\0';
            changeGameDir(player_score[i]->d_name, plid, 'Q');
        }
    }*/

    close(udp_fd);
    close(tcp_fd);
    exit(0);
}
//-----------------------------

int main(int argc, char *argv[]){
    pid_t pid;
    int n_trials = 0, line_number = 0;
    char command[COMMAND_SIZE];
    char plid[PLID_SIZE];
    char word_file[FILE_SIZE];
    char GSport[PORT_SIZE];
    char buffer[SEND_SIZE];
    strcpy(GSport, DEFAULT_GSport);
    strcpy(word_file, argv[1]);

    for(int i = 0; i < argc; i++){
        if(strcmp(argv[i], "-p") == 0){
            strcpy(GSport, argv[i+1]);
        }
        if(strcmp(argv[i], "-v") == 0){
            verbose = TRUE;
        }
    }

    signal(SIGINT, signalHandler);
    while(1){
        //UDP
        udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
        if(udp_fd == -1) exit(1);
        memset(&udp_hints, 0, sizeof(udp_hints));
        udp_hints.ai_family = AF_INET;
        udp_hints.ai_socktype = SOCK_DGRAM;
        udp_hints.ai_flags = AI_PASSIVE;
        errcode = getaddrinfo(NULL, GSport, &udp_hints, &udp_res);
        if(errcode != 0) exit(1);
        n = bind(udp_fd, udp_res->ai_addr, udp_res->ai_addrlen);
        if(n == -1) exit(1);

        //TCP
        tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
        if(tcp_fd == -1) exit(1);
        memset(&tcp_hints, 0, sizeof(tcp_hints));
        tcp_hints.ai_family = AF_INET;
        tcp_hints.ai_socktype = SOCK_STREAM;
        tcp_hints.ai_flags = AI_PASSIVE;
        errcode = getaddrinfo(NULL, GSport, &tcp_hints, &tcp_res);
        if((errcode) != 0) exit(1);
        n = bind(tcp_fd, tcp_res->ai_addr, tcp_res->ai_addrlen);
        if(n == -1) exit(1);
        if(listen(tcp_fd, 5) == -1) exit(1);

        //timeout -------------
        struct timeval timeout;      
        timeout.tv_sec = 600;
        timeout.tv_usec = 0;
        
        if (setsockopt (tcp_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout) < 0) printf("setsockopt failed\n");
        if (setsockopt (tcp_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof timeout) < 0) printf("setsockopt failed\n");

        if (setsockopt (udp_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout) < 0) printf("setsockopt failed\n");
        if (setsockopt (udp_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof timeout) < 0) printf("setsockopt failed\n");
        //---------------------

        if ((pid = fork()) == 0){
            //UDP
            while(1){
                memset(buffer, 0, strlen(buffer));
                memset(command, 0, strlen(command));
                addrlen = sizeof(addr);
                n = recvfrom(udp_fd, buffer, 128, 0, (struct sockaddr*) &addr, &addrlen);
                if(n == -1) exit(1);
                buffer[SEND_SIZE] = '\0';

                char *ptr = buffer;
                sscanf(buffer, "%s", command);
                ptr += strlen(command) + 1;
                
                if(strcmp(command, "SNG") == 0){
                    sscanf(ptr, "%s", plid);
                    start(plid, word_file, buffer, line_number);
                    line_number++;
                }

                else if(strcmp(command, "PLG") == 0){
                    char letter, c_trial[2];
                    sscanf(ptr, "%s %c %s", plid, &letter, c_trial);
                    int trial = atoi(c_trial);
                    play(plid, letter, trial);
                }

                else if(strcmp(command, "PWG") == 0){
                    char letter, c_trial[2], guess_word[WORD_SIZE];
                    sscanf(ptr, "%s %s %s", plid, guess_word, c_trial);
                    int trial = atoi(c_trial);
                    guess(plid, guess_word, trial);
                }

                else if(strcmp(command, "QUT") == 0){
                    sscanf(ptr, "%s", plid);
                    quit(plid);
                }

                if(strcmp(command, "REV") == 0){
                    sscanf(ptr, "%s", plid);
                    rev(plid);
                }

                if(verbose == TRUE){
                    verbosePrint(plid, command);
                }
            }
        }

        if (pid > 0){
            //TCP
            while(1){
                memset(buffer, 0, strlen(buffer));
                memset(command, 0, strlen(command));
                addrlen = sizeof(addr);
                newfd = accept(tcp_fd, (struct sockaddr*) &addr, &addrlen);
                if(newfd == -1) exit(1);
                n = read(newfd, buffer, 128);
                if(n == -1) exit(1);
                buffer[129] = '\0';
                char *ptr = buffer;
                sscanf(ptr, "%s", command);

                if(strcmp(command, "GSB") == 0){
                    scoreboard(pid);
                    if(verbose) verbosePrint("sb", command);
                }
                if(strcmp(command, "GHL") == 0){
                    ptr += strlen(command)+1;
                    sscanf(ptr, "%s", plid);
                    if(verbose) verbosePrint(plid, command);
                    hint(plid);
                }
                if(strcmp(command, "STA") == 0){
                    ptr += strlen(command)+1;
                    sscanf(ptr, "%s", plid);
                    if(verbose){
                        verbosePrint(plid, command);
                    }
                    state(plid);
                }
                close(newfd);
            }
            freeaddrinfo(tcp_res);
            close(tcp_fd);
        }
    }

    
}

void udpSendToClient(char buffer[], int c){
    n = sendto(udp_fd, buffer, c, 0, (struct sockaddr*) &addr, addrlen);
    if(n == -1) exit(1);
}

void tcpSendToClient(char buffer[], int c){
    while(c > 0){
        n = write(newfd, buffer, c);
        if(n == -1) exit(1);
        c -= n;
    }
}

void tcpSendFile(FILE *fp){
    char send[SEND_SIZE];
    int size_read;
    while(!feof(fp)){
        size_read = fread(send, 1, sizeof(send) - 1, fp);
        write(newfd, send, size_read);
    }
}

void start(char plid[], char word_file[], char buffer[], int line_number){
    char game_file[FILE_SIZE];
    int word_len;
    char word[30];
    char send[80];
    char line[50];
    int max_errors;

    if((strlen(plid) != 6) || (strlen(buffer) != 11)){
        sprintf(send, "RSG ERR\n");
        udpSendToClient("RSG ERR\n", strlen(send));
        return;
    } 

    sprintf(game_file, "GAME_%s.txt", plid);
    if(access(game_file, F_OK) == 0){
        FILE *fp = fopen(game_file, "r");
        if(fp == NULL) exit(1);

        int i = 0;
        for(; fgets(line, sizeof(line), fp) != NULL && i <= MAX_LINES; i++){
            if(i == 0){
                sscanf(line, "%s", word);
            }
        }
        
        if(i == 1){
            word_len = strlen(word);
            max_errors = getMaxErrors(word_len);
            sprintf(send, "RSG OK %d %d\n", word_len, max_errors);
            udpSendToClient(send, strlen(send));
        }
        else{
            strcpy(send, "RSG NOK\n");
            udpSendToClient(send, strlen(send));
        }
        fclose(fp);
        return;
    }
    else{
        FILE *fp = fopen(word_file, "r");
        if(fp == NULL) exit(1);

        char line[50];
        for(int i = 0; fgets(line, sizeof(line), fp) != NULL && i <= MAX_LINES; i++){
            if((i % MAX_LINES) == line_number){
                sscanf(line, "%s", word);
                break;
            }
        }
        
        fclose(fp);

        word_len = strlen(word);
        max_errors = getMaxErrors(word_len);

        FILE *fp_game = fopen(game_file, "w");
        fwrite(line, 1, strlen(line), fp);
        fclose(fp_game);

        sprintf(send, "RSG OK %d %d\n", word_len, max_errors);
        udpSendToClient(send, strlen(send));
        return;
    }
}

void play(char plid[], char letter, int trial){
    char game_file[FILE_SIZE];
    char send[80];
    char code[2];
    char play_wl[WORD_SIZE];
    int right_trials = 0, wrong_trials = 0, count_trials = 0;
    char word[WORD_SIZE];

    #include <ctype.h>
    if((strlen(plid) != 6) || (isalpha(letter) == 0)){
        sprintf(send, "RLG ERR %d\n", trial);
        udpSendToClient(send, strlen(send));
        return;
    }
    
    sprintf(game_file, "GAME_%s.txt", plid);

    if(access(game_file, F_OK) != 0){
        sprintf(send, "RLG ERR %d\n", trial);
        udpSendToClient(send, strlen(send));
        return;
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
                sprintf(send, "RLG DUP %d\n", trial);
                if(trial != i){
                    udpSendToClient(send, strlen(send));
                    fclose(fp);
                    return;
                }
            }
        }
        
        if(strcmp("T", code) == 0){
            if(strstr(word, play_wl) != NULL){
                for(int j = 0; j < strlen(word); j++){
                    char *let = play_wl;
                    if(word[j] == *let){
                        count_trials++;
                    }
                }
                right_trials++;
            }
            else{
                wrong_trials++;
            }
                
        }
        if(strcmp("G", code) == 0){
            if(strcmp(word, play_wl) == 0){
                right_trials++;
            }
            else{
                wrong_trials++;
            }
        }
    }

    if(i != trial){
        sprintf(send, "RLG INV %d\n", trial);
        udpSendToClient(send, strlen(send));
        fclose(fp);
        return;
    }
    if(strchr(word, letter) != NULL){
        int new_pos = 0;
        char positions[50] = "";
        for(int i = 0; i < strlen(word); i++){
            if(word[i] == letter){
                char aux[3];
                int x = i + 1;
                sprintf(aux, "%d ", x);
                strcat(positions, aux);
                new_pos++;
            }
        }
        positions[strlen(positions)-1] = '\0';

        int tr = count_trials + new_pos;
        
        if(tr == strlen(word)){
            char write[WORD_SIZE];
            sprintf(write, "T %c\n", letter);
            fileWrite(game_file, write, "a");
            scoreCreate(right_trials, wrong_trials, plid, game_file, word);
            changeGameDir(game_file, plid, 'W');
            sprintf(send, "RLG WIN %d\n", trial);
            udpSendToClient(send, strlen(send));
            fclose(fp);
            return;
        }
        else{
            char write[WORD_SIZE];
            sprintf(write, "T %c\n", letter);
            fileWrite(game_file, write, "a");
            sprintf(send, "RLG OK %d %d %s\n", (count_trials+wrong_trials), new_pos, positions);
            udpSendToClient(send, strlen(send));
            fclose(fp);
            return;
        }
    }
    else if(getMaxErrors(strlen(word)) <= (wrong_trials+1)){
        char write[WORD_SIZE];
        sprintf(write, "T %c\n", letter);
        fileWrite(game_file, write, "a");
        changeGameDir(game_file, plid, 'F');
        sprintf(send, "RLG OVR %d\n", trial);
        udpSendToClient(send, strlen(send));
        fclose(fp);
        return;
    }
    else if(getMaxErrors(strlen(word)) > wrong_trials++){
        char write[WORD_SIZE];
        sprintf(write, "T %c\n", letter);
        fileWrite(game_file, write, "a");
        sprintf(send, "RLG NOK %d\n", trial);
        udpSendToClient(send, strlen(send));
        fclose(fp);
        return;
    }

    fclose(fp);
    return;
}

void rev(char plid[]){
    char word[31], file[24], line[50], send[40];
    sprintf(file, "GAME_%s.txt", plid);
    FILE *fp = fopen(file, "r");
    if(fp == NULL) exit(1);
    fgets(line, sizeof(line), fp);
    sscanf(line, "%s", word);
    sprintf(send, "RRV %s\n", word);
    udpSendToClient(send, strlen(send));
    fclose(fp);
}

void scoreboard(int pid){
    char send[129];

    struct dirent **player_score;
    int m =  scandir("SCORES/", &player_score, NULL, alphasort);
    if(m < 0) perror("scandir");
    if(m > 12) m = 12;
    
    if(m == 2){
        sprintf(send, "RSB EMPTY\n");
        n = strlen(send);
        tcpSendToClient(send, strlen(send));
        return;
    }

    int total = 83 + 82 + (m-2) * 75;

    sprintf(send, "RSB OK TOPSCORES_%d.txt %d\n", pid, total);
    tcpSendToClient(send, strlen(send));

    sprintf(send, "\n-------------------------------- TOP 10 SCORES --------------------------------\n\n");
    send[86] = '\0';
    tcpSendToClient(send, strlen(send));

    sprintf(send, "    SCORE PLAYER     WORD                             GOOD TRIALS  TOTAL TRIALS\n\n");
    send[84] = '\0';
    tcpSendToClient(send, strlen(send));

    int count = 1;
    for(int i = (m-1); i >= 2; i--){
        char pl_line[85], word[31], spaces_1[40], spaces_2[15], aux[263];
        int score, plid, n_succ, n_trials;

        sprintf(aux, "SCORES/%s", player_score[i]->d_name);
        FILE *fp = fopen(aux, "r");
        if(fp == NULL) exit(1);
        fgets(pl_line, sizeof(pl_line), fp);
        sscanf(pl_line, "%d %d %s %d %d", &score, &plid, word, &n_succ, &n_trials);
        fclose(fp);

        char new_line[148];
        memset(spaces_1, ' ', (40 - strlen(word)));
        spaces_1[(40 - strlen(word))] = '\0';
        
        int n_sp;
        if(n_succ < 10) n_sp = 14;
        else n_sp = 13;
        memset(spaces_2, ' ', n_sp);
        spaces_2[n_sp] = '\0';
        
        if(count == 10) sprintf(new_line, "%d - %03d  %06d  %s%s%d%s%d\n", count, score, plid, word, spaces_1, n_succ, spaces_2, n_trials);
        else sprintf(new_line, " %d - %03d  %06d  %s%s%d%s%d\n", count, score, plid, word, spaces_1, n_succ, spaces_2, n_trials);
        count++;
        tcpSendToClient(new_line, strlen(new_line));
        free(player_score[i]);
    }
    free(player_score);

    return;
}

void guess(char plid[], char guess_word[], int trial){
    char game_file[FILE_SIZE], send[80], code[2];
    char play_wl[WORD_SIZE], word[WORD_SIZE], write[WORD_SIZE];
    int n_succ = 0, n_wrong = 0;

    if(strlen(plid) != 6 || strlen(guess_word) < 3){
        sprintf(send, "RWG ERR\n");
        udpSendToClient(send, strlen(send));
        return;
    }

    sprintf(game_file, "GAME_%s.txt", plid);
    if(access(game_file, F_OK) != 0){
        sprintf(send, "RWG ERR\n");
        udpSendToClient(send, strlen(send));
        return;
    }
    
    FILE *fp = fopen(game_file, "rw");
    if(fp == NULL) exit(1);
    char line[50];
    fgets(line, sizeof(line), fp);
    sscanf(line, "%s", word);

    int i = 1;
    int dup = 0, dup_trial = 0;
    for(; fgets(line, sizeof(line), fp) != NULL; i++){
        sscanf(line, "%s %s", code, play_wl);
        if(strcmp(code, "T") == 0){
            if(strstr(word, play_wl) != NULL){
                n_succ++;
            }
            else{
                n_wrong++;
            }
        }
        else if(strcmp(code, "G") == 0){
            if(strcmp(guess_word, play_wl) == 0){
                dup = 1;
                dup_trial = i;
            }
            else{
                n_wrong++;
            }
        }
    }

    if(strcmp(word, guess_word) == 0){
        sprintf(send, "RWG WIN %d\n", trial);

        sprintf(write, "G %s\n", guess_word);
        fileWrite(game_file, write, "a");

        udpSendToClient(send, strlen(send));
        fclose(fp);

        changeGameDir(game_file, plid, 'W');
        scoreCreate(n_succ, n_wrong, plid, game_file, word);
        return;
    }

    if(dup == 1){
        if((dup_trial + 1) == trial){
            sprintf(send, "RWG NOK %d\n", trial);
        }
        else{
            sprintf(send, "RWG DUP %d\n", trial);
        }
        udpSendToClient(send, strlen(send));
        fclose(fp);
        return;
    }
    if(i != trial){
        sprintf(send, "RWG INV\n");
        udpSendToClient(send, strlen(send));
        fclose(fp);
        return;
    }
    else{
        if(trial >= getMaxErrors(strlen(word))){
            sprintf(send, "RWG OVR %d\n", trial);

            sprintf(write, "G %s\n", guess_word);
            fileWrite(game_file, write, "a");

            changeGameDir(game_file, plid, 'F');
        }
        else{
            sprintf(send, "RWG NOK %d\n", trial);
            sprintf(write, "G %s\n", guess_word);
            fileWrite(game_file, write, "a");
        }
    }
    udpSendToClient(send, strlen(send));
    fclose(fp);
    return;
}

void quit(char plid[]){
    char game_file[FILE_SIZE];
    char send[80];
    char play_wl[WORD_SIZE];
    char word[WORD_SIZE];

    if(strlen(plid) != 6){
        sprintf(send, "RQT ERR\n");
        udpSendToClient(send, strlen(send));
    }

    sprintf(game_file, "GAME_%s.txt", plid);

    if(access(game_file, F_OK) != 0){
        sprintf(send, "RQT ERR\n");
        udpSendToClient(send, strlen(send));
    }
    else{
        FILE *fp = fopen(game_file, "r");
        if(fp == NULL) exit(1);

        char line[50];
        int i = 0;
        for(; fgets(line, sizeof(line), fp) != NULL; i++){
            ;
        }
    
        if(i == 1){
            sprintf(send, "RQT OK\n");
            udpSendToClient(send, strlen(send));
        }
        else{
            changeGameDir(game_file, plid, 'Q');
            sprintf(send, "RQT OK\n");
            udpSendToClient(send, strlen(send));
        }
        fclose(fp);
        return;
    }
}

void hint(char plid[]){
    char game_file[FILE_SIZE];
    char send[129];
    char file_name[FNAME_SIZE], path_file[40];
    char word[WORD_SIZE];
    struct stat sb;
    int size;

    sprintf(game_file, "GAME_%s.txt", plid);

    if(access(game_file, F_OK) != 0){
        sprintf(send, "RHL NOK\n");
        tcpSendToClient(send, strlen(send));
        return;
    }

    FILE *fp = fopen(game_file, "r");
    if(fp == NULL) exit(1);

    char line[60];
    fgets(line, sizeof(line), fp);
    sscanf(line, "%s %s", word, file_name);
    fclose(fp);

    sprintf(path_file, "HINTS/%s", file_name);
    fp = fopen(path_file, "r");
    if(fp == NULL) exit(1);

    fseek(fp, 0L, SEEK_END);
    size = ftell(fp);

    sprintf(send, "RHL OK %s %d\n", file_name, size);
    tcpSendToClient(send, strlen(send));

    fseek(fp, 0L, SEEK_SET);
    int size_read;
    while(size > 0){
        size_read = fread(send, 1, 128, fp);
        if(size_read == 0) break;
        send[129] = '\0';
        tcpSendToClient(send, size_read);
        size -= size_read;
    }

    fclose(fp);
}

void state(char plid[]){
    char game_file[FILE_SIZE], send[305], line[50], word[30], hint_file[24], path[30], Fname[285], state_file[17];
    FILE *fp;

    sprintf(game_file, "GAME_%s.txt", plid);

    if(access(game_file, F_OK) != 0){
        struct dirent **player_score;
        sprintf(path, "GAMES/%s/", plid);
        int m =  scandir(path, &player_score, NULL, alphasort);
        if(m < 0) perror("scandir");

        if(m == 0){
            sprintf(send, "RST NOK\n");
            tcpSendToClient(send, strlen(send));
            memset(send, 0, strlen(send));
        }
        else{
            sprintf(Fname, "%s%s", path, player_score[m-1]->d_name);
            fp = fopen(Fname, "r");
            if(fp == NULL) exit(1);

            char code, wl[30];
            int count_words = 0, count_letters = 0, word_size = 0;
            for(int i = 0; fgets(line, sizeof(line), fp) != NULL; i++){
                if(i == 0){
                    sscanf(line, "%s %s", word, hint_file);
                }
                else{
                    sscanf(line, "%c %s", &code, wl);
                    if(code == 'T') count_letters++;
                    if(code == 'G'){
                        count_words++;
                        word_size += strlen(wl);
                    }
                }
            }

            int size = 38 + 6 + strlen(word) + 13 + strlen(hint_file) + 20 + (16 * count_letters) + (13 * count_words) + word_size + 18;
            int transactions = count_letters + count_words;

            sprintf(state_file, "STATE_%s.txt", plid);
            state_file[17] = '\0';
            
            sprintf(send, "RST FIN %s %d\n", state_file, size);
            tcpSendToClient(send, strlen(send));
            
            sprintf(send, "Last finalized game for player %s\n", plid);
            send[39] = '\0';
            tcpSendToClient(send, strlen(send));

            sprintf(send, "Word: %s, Hint file: %s\n", word, hint_file);
            send[21+strlen(word)+strlen(hint_file)] = '\0';
            tcpSendToClient(send, strlen(send));

            sprintf(send, "Transactions found: %d\n", transactions);
            send[24] = '\0';
            tcpSendToClient(send, strlen(send));

            fseek(fp, 0, SEEK_SET);
            fgets(line, sizeof(line), fp);
            for(int i = 0; fgets(line, sizeof(line), fp) != NULL; i++){
                sscanf(line, "%c %s", &code, wl);
                if(code == 'G'){
                    sprintf(send, "Word guess: %s\n", wl);
                    send[14+strlen(wl)] = '\0';
                    tcpSendToClient(send, strlen(send));
                }
                else if(code == 'T'){
                    sprintf(send, "Letter trial: %s\n", wl);
                    send[16+strlen(wl)] = '\0';
                    tcpSendToClient(send, strlen(send));
                }
            }

            if(strstr(Fname, "W") != NULL){
                sprintf(send, "Termination: WIN\n");
                send[18] = '\0';
                tcpSendToClient(send, strlen(send));
            }
            else if(strstr(Fname, "F") != NULL){
                sprintf(send, "Termination: FAIL\n");
                send[19] = '\0';
                tcpSendToClient(send, strlen(send));
            }
            else{
                sprintf(send, "Termination: QUIT\n");
                send[19] = '\0';
                tcpSendToClient(send, strlen(send));
            }
        }
    }
    else{
        fp = fopen(game_file, "r");
        if(fp == NULL) exit(1);
        int n_trues = 0, n_falses = 0;
        char code, wl[30];
        int count_words = 0, count_letters = 0, word_size = 0;

        int i = 0;
        for(; fgets(line, sizeof(line), fp) != NULL; i++){
            if(i == 0){
                sscanf(line, "%s %s", word, hint_file);
            }
            else{
                sscanf(line, "%c %s", &code, wl);
                if(code == 'T') {
                    if(strstr(word, wl)){
                        n_trues++;
                    }
                    else{
                        n_falses++;
                    }
                }
                if(code == 'G'){
                    count_words++;
                    word_size += strlen(wl);
                }
            }
        }
        int transactions = i-1;

        int size = 36 + 22 + 23*(n_trues+n_falses) + count_words*transactions + transactions + word_size + 15 + strlen(word);

        sprintf(Fname, "STATE_%s.txt", plid);
        char aux[5];
        sprintf(aux, "%d", size);
        sprintf(send, "RST ACT %s %d\n", Fname, size);
        tcpSendToClient(send, strlen(send));

        sprintf(send, "\nActive game found for player %s\n", plid);
        tcpSendToClient(send, strlen(send));

        sprintf(send, "Transactions found: %d\n", transactions);
        tcpSendToClient(send, strlen(send));

        fseek(fp, 0, SEEK_SET);
        char word_so_far[WORD_SIZE];
        memset(word_so_far, '_', strlen(word));

        if(transactions > 0){
            fgets(line, sizeof(line), fp);
            sscanf(line, "%s", word);
            for(i = 0; fgets(line, sizeof(line), fp) != NULL; i++){
                sscanf(line, "%c %s", &code, wl);
                if (code == 'T'){
                    if(strstr(word, wl)){
                        sprintf(send, "Letter trial: %s - TRUE\n", wl);
                        tcpSendToClient(send, strlen(send));
                        for(int j = 0; j < strlen(word); j++){
                            char *let = wl;
                            if(word[j] == *let){
                                word_so_far[j] = *let;
                            }
                        }
                    }
                    else{
                        sprintf(send, "Letter trial: %s - FALSE\n", wl);
                        tcpSendToClient(send, strlen(send));
                    }
                }
                else if(code == 'G'){
                    sprintf(send, "Word guess: %s - FALSE\n", wl);
                    tcpSendToClient(send, strlen(send));
                }
            }
        }
        
        word_so_far[strlen(word)] = '\0';
        sprintf(send, "Solved so far: %s\n\n", word_so_far);
        send[17+strlen(word_so_far)] = '\0';
        tcpSendToClient(send, strlen(send));
    }
    fclose(fp);
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
    if(fp == NULL) exit(1);
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

    sprintf(new_filename, "GAMES/%s/%04d%02d%02d_%02d%02d%02d_%c.txt", plid, t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, code);
    int m = rename(filename, new_filename);
}

void scoreCreate(int n_succ, int n_wrong, char plid[], char filename[], char word[]){
    int total_trials = n_succ + n_wrong;
    int score = (n_succ * 100) / total_trials;
    
    struct stat attr;
    struct tm *t;
    stat(filename, &attr);
    t = gmtime(&attr.st_mtime);

    char scorefile[40];
    sprintf(scorefile, "SCORES/%03d_%s_%04d%02d%02d_%02d%02d%02d.txt", score, plid, t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);

    FILE *fp = fopen(scorefile, "w");
    if(fp == NULL) exit(1);
    
    char write[30];
    sprintf(write, "%d %s %s %d %d", score, plid, word, n_succ, total_trials);

    fwrite(write, 1, strlen(write), fp);
    fclose(fp);
}

void verbosePrint(char plid[], char command[]){
    char host[NI_MAXHOST],service[NI_MAXSERV];
    if((errcode = getnameinfo((struct sockaddr *)&addr, addrlen, host, sizeof(host), service, sizeof(service), 0)) != 0) fprintf(stderr,"error: getnameinfo: %s\n", gai_strerror(errcode));
    else{
        if(strcmp("sb", plid) != 0) printf("\n\nCommand: %s\nIP: %s\nPort: %s\n", command, host, service);
        else printf("\n\nPlayer ID: %s\nCommand: %s\nIP: %s\nPort: %s\n", plid, command, host, service);
    } 

}
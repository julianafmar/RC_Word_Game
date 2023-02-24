#include "player.h"

#include <signal.h>
void signalHandler(int sig){
    signal(sig, SIG_IGN);
    quit();
    exit(0);
}

int main(int argc, char *argv[]){
    strcpy(GSport, DEFAULT_GSport);

    /*Get default IP Address*/
    char hostbuffer[256];
    char *IPbuffer;
    struct hostent *host_entry;
    int hostname;
    hostname = gethostname(hostbuffer, sizeof(hostbuffer));
    if(hostname == -1) perror("gethostname");
    host_entry = gethostbyname(hostbuffer);
    if(host_entry == NULL) perror("gethostbyname");
    IPbuffer = inet_ntoa(*((struct in_addr*)host_entry->h_addr_list[0]));
    if(IPbuffer == NULL) perror("inet_ntoa");
    strcpy(GSIP, IPbuffer);

    /*Get port number and IP Address, if they were given via terminal*/
    for(int i = 0; i < argc - 1; i++){
        if(strcmp(argv[i], "-p") == 0){
            strcpy(GSport, argv[i+1]);
        }
        if(strcmp(argv[i], "-n") == 0){
            strcpy(GSIP, argv[i+1]);
        }
    }

    char input[INPUT_SIZE];

    while(1){
        signal(SIGINT, signalHandler);
        printf("Insert command: ");
        /*Get standard input*/
        fgets(input, INPUT_SIZE, stdin);

        char command[4], plid[PLID_SIZE], let, word[WORD_SIZE];
        char *ptr = input;
        sscanf(ptr, "%s", command);
        ptr = ptr + strlen(command) + 1;

        if(strcmp(command, "start") == 0 || strcmp(command, "sg") == 0){
            sscanf(ptr, "%s", plid);
            memset(word_spaces, 0 , 30);
            start(plid);
        } 
        else if(strcmp(command, "play") == 0 || strcmp(command, "pl") == 0){
            sscanf(ptr, "%c", &let);
            play(let);
        }
        else if(strcmp(command, "guess")== 0 || strcmp(command, "gw") == 0){
            sscanf(ptr, "%s", word); 
            guess(word);
        }
        else if(strcmp(command, "scoreboard") == 0 || strcmp(command, "sb") == 0){
            scoreboard();
        }
        else if(strcmp(command, "hint") == 0 || strcmp(command, "h") == 0){
            hint();
        }
        else if(strcmp(command, "state") == 0 || strcmp(command, "st") == 0){
            state();
        }
        else if(strcmp(command, "quit") == 0){
            quit();
        }
        else if(strcmp(command, "exit") == 0){
            quit();
            break;
        }
        else if(strcmp(command, "rev") == 0){
            rev();
        }
        else{
            printf("This command doesn't exist.\n");
        }

        memset(input, 0, strlen(input));
        memset(ptr, 0, strlen(ptr));
        letter_try = '\0';
    }
    return 0;
}

/*Format message to be send to server for start command*/
void start(char plid[]){
    char send[INPUT_SIZE];
    n_trials = 1;
    strcpy(id, plid);
    sprintf(send, "SNG %s\n", plid);
    communication_udp(send);
}

/*Format message to be send to server for play command*/
void play(char letter){
    char send[INPUT_SIZE];
    sprintf(send, "PLG %s %c %d\n", id,  letter, n_trials);
    letter_try = letter;
    communication_udp(send);
}

/*Format message to be send to server for guess command*/
void guess(char word[]){
    char send[INPUT_SIZE];
    sprintf(send, "PWG %s %s %d\n", id, word, n_trials);
    communication_udp(send);
}

/*Format message to be send to server for scoreboard command*/
void scoreboard(){
    char send[INPUT_SIZE];
    sprintf(send, "GSB\n");
    communication_tcp(send);
}

/*Format message to be send to server for hint command*/
void hint(){
    char send[INPUT_SIZE];
    sprintf(send, "GHL %s\n", id);
    communication_tcp(send);
}

/*Format message to be send to server for state command*/
void state(){
    char send[INPUT_SIZE];
    sprintf(send, "STA %s\n", id);
    communication_tcp(send);
}

/*Format message to be send to server for quit command*/
void quit(){
    char send[INPUT_SIZE];
    sprintf(send, "QUT %s\n", id);
    communication_udp(send);
}

/*Format message to be send to server for rev command*/
void rev(){
    char send[INPUT_SIZE];
    sprintf(send, "REV %s\n", id);
    communication_udp(send);
}

/*Send message to server via udp socket*/
void communication_udp(char *send){
    char buffer[BUFFER_SIZE];
    udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(udp_fd == -1) exit(1);
    memset(&udp_hints, 0, sizeof(udp_hints));

    /*Timeout*/
    struct timeval timeout;
    timeout.tv_sec = TIMER_VALUE;
    timeout.tv_usec = 0;
    if (setsockopt(udp_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) printf("setsockopt failed\n");
    if (setsockopt(udp_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) printf("setsockopt failed\n");

    udp_hints.ai_family = AF_INET;
    udp_hints.ai_socktype = SOCK_DGRAM;

    errcode = getaddrinfo(GSIP, GSport, &udp_hints, &udp_res);
    if(errcode != 0) exit(1);
    n = sendto(udp_fd, send, strlen(send), 0, udp_res->ai_addr, udp_res->ai_addrlen);
    if(n == -1) exit(1);

    addrlen = sizeof(addr);
    n = recvfrom(udp_fd, buffer, 128, 0, (struct sockaddr*) &addr, &addrlen);
    if(n == -1) exit(1);
    buffer[129] = '\0';

    received_udp(buffer);

    freeaddrinfo(udp_res);
    close(udp_fd);
}

/*Send message to server via tcp socket*/
void communication_tcp(char *send){
    char buffer[BUFFER_SIZE];
    tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(tcp_fd == -1) exit(1);
    memset(&tcp_hints, 0, sizeof(tcp_hints));

    /*Timemout*/
    struct timeval timeout;      
    timeout.tv_sec = TIMER_VALUE;
    timeout.tv_usec = 0;
    if (setsockopt(tcp_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) printf("setsockopt failed\n");
    if (setsockopt(tcp_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) printf("setsockopt failed\n");

    tcp_hints.ai_family = AF_INET;
    tcp_hints.ai_socktype = SOCK_STREAM;

    errcode = getaddrinfo(GSIP, GSport, &tcp_hints, &tcp_res);
    if(errcode != 0) exit(1);
    
    n = connect(tcp_fd, tcp_res->ai_addr, tcp_res->ai_addrlen);
    if(n == -1) exit(1);

    n = write(tcp_fd, send, strlen(send));
    if(n == -1) exit(1);

    n = read(tcp_fd, buffer, 128);
    if(n == -1) exit(1);
    buffer[129] = '\0';
    if(n < 128 && n != 0){
        char *buf = buffer;
        buf += n;
        int m = n;
        n = read(tcp_fd, buf, 128 - m);
        if(n == -1) exit(1);
        buffer[129] = '\0';
        n = 128;
    }
    
    received_tcp(buffer);
    freeaddrinfo(tcp_res);
    close(tcp_fd);
}

/*Analized what has been received via udp socket and answers to player*/
void received_udp(char *received){
    char command[4], status[4];
    sscanf(received, "%s", command);
    received = received + strlen(command) + 1;

    /*Handles start command answers*/
    if(strcmp(command, "RSG") == 0){
        sscanf(received, "%s", status);
        received = received + strlen(status) + 1;
        if(strcmp(status, "NOK") == 0){
            printf("You already have an ongoing game.\n");
        }
        else if(strcmp(status, "OK") == 0){
            sscanf(received, "%d %d", &n_letters, &max_errors);
            memset(word_spaces, *UNDERSCORE, n_letters);
            printf("New game started. Guess %d letter word (max %d errors): %s\n", n_letters, max_errors, word_spaces);
        }
        else if(strcmp(status, "ERR") == 0){
            printf("RSG ERR\n");
        }
    }
    
    /*Handles play command answers*/
    else if(strcmp(command, "RLG") == 0){
        sscanf(received, "%s", status);
        received = received + strlen(status) + 1;
        if(strcmp(status, "OK") == 0){
            char n_str[3], trial[3];
            sscanf(received, "%s %s", trial, n_str);
            received = received + strlen(trial) + strlen(n_str) + 2;
            n_trials++;
            int n = atoi(n_str);
            for(int i = 0; i < n; i++){
                sscanf(received, "%s", n_str);
                received = received + strlen(n_str) + 1;
                word_spaces[atoi(n_str)-1] = letter_try;
            }
            printf("Word: %s\n", word_spaces);
        }
        else if(strcmp(status, "WIN") == 0){
            printf("You won!\n");
        }
        else if(strcmp(status, "DUP") == 0){
            printf("You already tried this letter.\n");
        }
        else if(strcmp(status, "NOK") == 0){
            n_trials++;
            printf("Ups! This letter is not part of the word.\n");
        }
        else if(strcmp(status, "OVR") == 0){
            n_trials++;
            printf("Game over.\n");
        }
        else if(strcmp(status, "INV") == 0){
            printf("The trial number is invalid.\n");
        }
        else if(strcmp(status, "ERR") == 0){
            printf("RLG ERR\n");
        }
    }

    /*Handles guess command answers*/
    else if(strcmp(command, "RWG") == 0){
        sscanf(received, "%s", status);
        if(strcmp(status, "WIN") == 0){
            printf("You won!\n");
        }
        else if(strcmp(status, "DUP") == 0){
            printf("You already tried this word.\n");
        }
        else if(strcmp(status, "NOK") == 0){    
            n_trials++;
            printf("That's not the right word.\n");
        }
        else if(strcmp(status, "OVR") == 0){
            printf("Game over.");
        }
        else if(strcmp(status, "INV") == 0){
            printf("The trial number is invalid.\n");
        }
        else if(strcmp(status, "ERR") == 0){
            printf("RWG ERR\n");
        }
    }

    /*Handles quit and exit commands answers*/
    else if(strcmp(command, "RQT") == 0){
        sscanf(received, "%s", status);
        if(strcmp(status, "OK") == 0){
            printf("Goodbye!\n");
            n_trials = 0;
        }
        else if(strcmp(status, "NOK") == 0){
            printf("You don't have an ongoing game.\n");
        }
        else if(strcmp(status, "ERR") == 0){
            printf("RQT ERR\n");
        }
    }

    /*Handles rev command answers*/
    else if(strcmp(command, "RRV") == 0){
        char word[WORD_SIZE];
        sscanf(received, "%s", word);
        printf("%s\n", word);
    }
}

/*Analized what has been received via udp socket and answers to player*/
void received_tcp(char *received){
    char buffer[BUFFER_SIZE];
    char command[3];
    char status[6];
    char Fname[24];
    char Fsize[28];
    
    sscanf(received, "%s", command);

    /*Handles scoreboard command answers*/
    if(strcmp(command, "RSB") == 0){
        received += 4;
        sscanf(received, "%s", status);
        if(strcmp(status, "EMPTY") == 0){
            printf("There is no scoreboard.\n");
        }
        if(strcmp(status, "OK") == 0){
            received += strlen(status) + 1;
            sscanf(received, "%s", Fname);

            received += strlen(Fname) + 1;
            sscanf(received, "%s", Fsize);

            received += strlen(Fsize) + 1;

            FILE *fp = fopen(Fname, "w");
            if(fp == NULL){
                printf("There was a problem opening a file.\n");
                return;
            }

            n -= (9 + strlen(Fname) + strlen(Fsize));
            int size = atoi(Fsize);
            size -= n;
            fwrite(received, 1, n, fp);
            while(n > 0 && size > 0){
                n = read(tcp_fd, buffer, 128);
                buffer[129] = '\0';
                fwrite(buffer, 1, n, fp);
                if(n == -1) exit(1);
                size -= n;
            }
            fclose(fp);

            fp = fopen(Fname, "r");
            if(fp == NULL){
                printf("There was a problem opening a file.\n");
                return;
            }
            char c = fgetc(fp);
            while(c != EOF){
                printf("%c", c);
                c = fgetc(fp);
            }
            fclose(fp);
        }
    }

    /*Handles hint command answers*/
    else if(strcmp(command, "RHL") == 0){
        received += 4;
        sscanf(received, "%s", status);
        if(strcmp(status, "NOK") == 0){
            printf("There was a problem.\n");
        }
        int r = 0;
        if(strcmp(status, "OK") == 0){
            received += strlen(status) + 1;
            sscanf(received, "%s", Fname);

            received += strlen(Fname) + 1;
            sscanf(received, "%s", Fsize);

            received += strlen(Fsize) + 1;
        
            FILE *fp = fopen(Fname, "w");
            if(fp == NULL){
                printf("There was a problem opening a file.\n");
                return;
            }
            n = n - (9 + strlen(Fname) + strlen(Fsize));
            int size = atoi(Fsize);
            size -= n;
            fwrite(received, 1, n, fp);
            while(n > 0 && size > 0){
                n = read(tcp_fd, buffer, 128);
                if(n == -1) exit(1);
                buffer[129] = '\0';
                fwrite(buffer, 1, n, fp);
                if(n == -1) exit(1);
                size -= n;
            }
            fclose(fp);
            printf("The filename is %s and its size is %s.\n", Fname, Fsize);
        }
        if(strcmp(status, "NOK") == 0){
            printf("There is no file to be send.\n");
        }
    }

    /*Handles state command answers*/
    else if(strcmp(command, "RST") == 0){
        received += 4;
        sscanf(received, "%s", status);
        if(strcmp(status, "ACT") == 0){
            received += strlen(status) + 1;
            sscanf(received, "%s", Fname);

            received += strlen(Fname) + 1;
            sscanf(received, "%s", Fsize);
            received += strlen(Fsize) + 1;

            printf("The filename is %s and its size is %s. File's content is:\n", Fname, Fsize);
            
            FILE *fp = fopen(Fname, "w");
            if(fp == NULL){
                printf("There was a problem opening a file.\n");
                return;
            }

            n -= (9 + strlen(Fname) + strlen(Fsize));
            int size = atoi(Fsize);
            if((size - n) < 0) n = size;
            size -= n;
            fwrite(received, 1, n, fp);
            while(n > 0 && size > 0){
                memset(buffer, 0, strlen(buffer));
                n = read(tcp_fd, buffer, 128);
                buffer[129] = '\0';
                fwrite(buffer, 1, n, fp);
                if(n == -1) exit(1);
                size -= n;
            }
            fclose(fp);

            fp = fopen(Fname, "r");
            if(fp == NULL){
                printf("There was a problem opening a file.\n");
                return;
            }
            char c = fgetc(fp);
            int x = 0;
            while(c != EOF){
                printf("%c", c);
                c = fgetc(fp);
                x++;
            }
            fclose(fp);
        }
        else if(strcmp(status, "FIN") == 0){
            received += strlen(status) + 1;
            sscanf(received, "%s", Fname);

            received += strlen(Fname) + 1;
            sscanf(received, "%s", Fsize);
            received += strlen(Fsize) + 1;

            printf("The filename is %s and its size is %s. File's content is:\n", Fname, Fsize);

            FILE *fp = fopen(Fname, "w");
            if(fp == NULL){
                printf("There was a problem opening a file.\n");
                return;
            }

            n -= (9 + strlen(Fname) + strlen(Fsize));
            int size = atoi(Fsize);
            size -= n;
            fwrite(received, 1, n, fp);
            while(n > 0 && size > 0){
                n = read(tcp_fd, buffer, 128);
                buffer[129] = '\0';
                fwrite(buffer, 1, n, fp);
                if(fp == NULL){
                    printf("There was a problem opening a file.\n");
                    return;
                }
                size -= n;
            }
            fclose(fp);

            fp = fopen(Fname, "r");
            if(fp == NULL){
                printf("There was a problem opening a file.\n");
                return;
            }
            char c = fgetc(fp);
            while(c != EOF){
                printf("%c", c);
                c = fgetc(fp);
            }
            fclose(fp);
        }
        else if(strcmp(status, "NOK") == 0){
            printf("There are no games.\n");
        }
    }
}
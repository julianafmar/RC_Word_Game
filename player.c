#include "player.h"

int main(int argc, char *argv[]){
    strcpy(GSport, DEFAULT_GSport);

    /*char hostbuffer[256];
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
    printf("%s\n", GSIP);*/

     int fdip;
    struct ifreq ifr;

    fdip = socket(AF_INET, SOCK_DGRAM, 0);
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);
    ioctl(fdip, SIOCGIFADDR, &ifr);
    close(fdip);
    strcpy(GSIP, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

    for(int i = 0; i < argc - 1; i++){
        //ver o -1
        if(strcmp(argv[i], "-p") == 0){
            strcpy(GSport, argv[i+1]);
        }
        if(strcmp(argv[i], "-n") == 0){
            strcpy(GSIP, argv[i+1]);
        }
    }

    char input[INPUT_SIZE];

    while(1){
        printf("Insert command: ");
        fgets(input, INPUT_SIZE, stdin);

		char *token_list[5];
		char *tok = strtok(input, " \n");
		for (int i = 0; tok != NULL && i < 5; i++){
			token_list[i] = tok;
			tok = strtok(NULL, " \n");
		}
        if(strcmp(token_list[0], "start") == 0 || strcmp(token_list[0], "sg") == 0){
            memset(word_spaces, 0 , 30);
            start(token_list[1]);
        } 
        else if(strcmp(token_list[0], "play") == 0 || strcmp(token_list[0], "pl") == 0){
            play(*token_list[1]);
        }
        else if(strcmp(token_list[0], "guess")== 0 || strcmp(token_list[0], "gw") == 0){
            guess(token_list[1]);
        }
        else if(strcmp(token_list[0], "scoreboard") == 0 || strcmp(token_list[0], "sb") == 0){
            scoreboard();
        }
        else if(strcmp(token_list[0], "hint") == 0 || strcmp(token_list[0], "h") == 0){
            hint();
        }
        else if(strcmp(token_list[0], "state") == 0 || strcmp(token_list[0], "st") == 0){
            state();
        }
        else if(strcmp(token_list[0], "quit") == 0){
            quit();
        }
        else if(strcmp(token_list[0], "exit") == 0){
            quit();
            break;
        }
        else if(strcmp(token_list[0], "rev") == 0){
            rev();
        }
        else{
            printf("This command doesn't exist.\n");
        }

        memset(input, 0, strlen(input));
        letter_try = '\0';
    }
    return 0;
}

void start(char plid[]){
    char send[INPUT_SIZE];
    n_trials++; //qnd ja existe jogo isto fica errado :/
    strcpy(id, plid);
    sprintf(send, "SNG %s\n", plid);
    communication_udp(send);
}

void play(char letter){
    char send[INPUT_SIZE];
    sprintf(send, "PLG %s %c %d\n", id, letter, n_trials);
    letter_try = letter;
    communication_udp(send);
}

void guess(char word[]){
    char send[INPUT_SIZE];
    sprintf(send, "PWG %s %s %d\n", id, word, n_trials);
    communication_udp(send);
}

void scoreboard(){
    char send[INPUT_SIZE];
    sprintf(send, "GSB\n");
    communication_tcp(send);
}

void hint(){
    char send[INPUT_SIZE];
    sprintf(send, "GHL %s\n", id);
    communication_tcp(send);
}

void state(){
    char send[INPUT_SIZE];
    sprintf(send, "STA %s\n", id);
    communication_tcp(send);
}

void quit(){
    char send[INPUT_SIZE];
    sprintf(send, "QUT %s\n", id);
    communication_udp(send);
}

void rev(){
    char send[INPUT_SIZE];
    sprintf(send, "REV %s\n", id);
    communication_udp(send);
}

void communication_udp(char *send){
    char buffer[BUFFER_SIZE];
    udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(udp_fd == -1) exit(1);
    memset(&udp_hints, 0, sizeof(udp_hints));

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

void communication_tcp(char *send){
    char buffer[BUFFER_SIZE];
    tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(tcp_fd == -1) exit(1);
    memset(&tcp_hints, 0, sizeof(tcp_hints));

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
    printf("bb %s\n", buffer);
    
    received_tcp(buffer);
    freeaddrinfo(tcp_res);
    close(tcp_fd);
}

void received_udp(char *received){
    char *token_list[35];
    char *tok = strtok(received, " \n");
    for (int i = 0; tok != NULL && i < INPUT_SIZE; i++){
        token_list[i] = tok;
        tok = strtok(NULL, " \n");
    }
    if(strcmp(token_list[0], "RSG") == 0){
        if(strcmp(token_list[1], "NOK") == 0){
            printf("You already have an ongoing game.\n");
        }
        else if(strcmp(token_list[1], "OK") == 0){
            max_errors = atoi(token_list[3]);
            n_letters = atoi(token_list[2]);
            memset(word_spaces, *UNDERSCORE, n_letters);
            printf("New game started. Guess %s letter word (max %s errors): %s\n", token_list[2], token_list[3], word_spaces);
        }
        else if(strcmp(token_list[1], "ERR") == 0){
            printf("RSG ERR\n");
        }
    }
    else if(strcmp(token_list[0], "RLG") == 0){
        if(strcmp(token_list[1], "OK") == 0){
            n_trials++;
            int n = atoi(token_list[3]);
            for(int i = 0; i < n; i++){
                word_spaces[atoi(token_list[4+i])-1] = letter_try;
            }
            printf("Word: %s\n", word_spaces);
        }
        else if(strcmp(token_list[1], "WIN") == 0){
            printf("You won!\n");
        }
        else if(strcmp(token_list[1], "DUP") == 0){
            printf("You already tried this letter.\n");
        }
        else if(strcmp(token_list[1], "NOK") == 0){
            n_trials++;
            printf("Ups! This letter is not part of the word.\n");
        }
        else if(strcmp(token_list[1], "OVR") == 0){
            n_trials++;
            printf("Game over.\n");
        }
        else if(strcmp(token_list[1], "INV") == 0){
            printf("The trial number is invalid.\n");
        }
        else if(strcmp(token_list[1], "ERR") == 0){
            printf("RLG ERR\n");
        }
    }
    else if(strcmp(token_list[0], "RWG") == 0){
        if(strcmp(token_list[1], "WIN") == 0){
            printf("You won!\n");
        }
        else if(strcmp(token_list[1], "DUP") == 0){
            printf("You already tried this word.\n");
        }
        else if(strcmp(token_list[1], "NOK") == 0){    
            n_trials++;
            printf("That's not the right word.\n");
        }
        else if(strcmp(token_list[1], "OVR") == 0){
            printf("Game over.");
        }
        else if(strcmp(token_list[1], "INV") == 0){
            printf("The trial number is invalid.\n");
        }
        else if(strcmp(token_list[1], "ERR") == 0){
            printf("RWG ERR\n");
        }
    }
    else if(strcmp(token_list[0], "RQT") == 0){
        if(strcmp(token_list[1], "OK") == 0){
            printf("Goodbye!\n");
        }
        else if(strcmp(token_list[1], "ERR") == 0){
            printf("RQT ERR\n");
        }
    }
    else if(strcmp(token_list[0], "RRV") == 0){
        printf("%s", token_list[1]);
    }
}

void received_tcp(char *received){
    char buffer[BUFFER_SIZE];
    char command[3];
    char status[6];
    char Fname[24];
    char Fsize[28];
    
    sscanf(received, "%s", command);
    printf("com %s\n", command);
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
            if(fp == NULL) exit(1);

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
            if(fp == NULL) exit(1);
            char c = fgetc(fp);
            while(c != EOF){
                printf("%c", c);
                c = fgetc(fp);
            }
            fclose(fp);
        }
    }
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
            if(fp == NULL) exit(1);
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
    else if(strcmp(command, "RST") == 0){
        received += 4;
        sscanf(received, "%s", status);
        if(strcmp(status, "ACT") == 0){
            received += strlen(status) + 1;
            sscanf(received, "%s", Fname);

            received += strlen(Fname) + 1;
            sscanf(received, "%s", Fsize);
            received += strlen(Fsize) + 1;

            FILE *fp = fopen(Fname, "w");
            if(fp == NULL) exit(1);

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

            printf("The filename is %s and its size is %s. File's content is:\n", Fname, Fsize);

            fp = fopen(Fname, "r");
            if(fp == NULL) exit(1);
            char c = fgetc(fp);
            while(c != EOF){
                printf("%c", c);
                c = fgetc(fp);
            }
            fclose(fp);
        }
        else if(strcmp(status, "FIN") == 0){
            received += strlen(status) + 1;
            sscanf(received, "%s", Fname);

            received += strlen(Fname) + 1;
            sscanf(received, "%s", Fsize);
            received += strlen(Fsize) + 1;

            FILE *fp = fopen(Fname, "w");
            if(fp == NULL) exit(1);

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

            printf("The filename is %s and its size is %s. File's content is:\n", Fname, Fsize);

            fp = fopen(Fname, "r");
            if(fp == NULL) exit(1);
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
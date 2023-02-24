#ifndef GS_H
#define GS_H

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
#include <ctype.h>

#define TRUE 1
#define FALSE 0

#define DEFAULT_GSport "58033"
#define PORT_SIZE 16
#define MAX_LINES 26
#define FILE_SIZE 30
#define WORD_SIZE 31
#define FNAME_SIZE 24
#define SEND_SIZE 129
#define COMMAND_SIZE 4
#define LINE_SIZE 128
#define PATH_SIZE 40
#define CONTENT_FILE 305
#define PLID_SIZE 7
#define TIMER_VALUE 120

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

#endif
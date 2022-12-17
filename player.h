#ifndef PLAYER_H
#define PLAYER_H

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

#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>

#define UNDERSCORE "_"
#define WHITESPACE " "
#define DEFAULT_GSport "58033"
#define INPUT_SIZE 30
#define PORT_SIZE 16
#define IP_SIZE 64
#define PLID_SIZE 7
#define WORD_SIZE 31
#define BUFFER_SIZE 129

int tcp_fd, udp_fd, errcode;
ssize_t n;
socklen_t addrlen;
struct addrinfo udp_hints, tcp_hints, *udp_res, *tcp_res;
struct sockaddr_in addr;

char id[PLID_SIZE];
int n_trials = 0;
int max_errors;
int n_letters;
char word_spaces[WORD_SIZE];
char letter_try;
char GSport[PORT_SIZE], GSIP[IP_SIZE];

void start(char plid[]);
void play(char letter);
void guess(char word[]);
void scoreboard();
void hint();
void state();
void quit();
void rev();
void communication_udp(char *send);
void communication_tcp(char *send);
void received_udp(char *received);
void received_tcp(char *received);

#endif
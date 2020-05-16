#ifndef PREPROC_H
#define PREPROC_H

#define or              ||
#define and             &&
#define true            1
#define false           0
#define not             !
#define new(a,b)        (calloc(a, sizeof(b)))
#define for_i_up_to(x)  for(int i=0; i<x; i++)
#define for_j_up_to(x)  for(int j=0; j<x; j++)
#define check_file(x)   if(x==NULL){printf("failed to open file\n"); exit(-1);}
#define c_size          sizeof(char)
#define min(a, b)       a > b ? b : a
#define max(a, b)       a > b ? a : b
#define string          char*
#define forewer         while(1)
#define safe_call(X)    if (X < 0) error("operation failed");
#define not_null(X)     if (X == NULL) error("null");
#define not_null_m(X, msg)     if (X == NULL) error(msg);
#define print       printf
// #define _POSIX_C_SOURCE 1
// #define _XOPEN_SOURCE 700
#define DEBUG           true
#define LOG             if(DEBUG)printf
#define bool            short

#define SERVER_NAME             "game_server"
#define SERVER_CLIENTS_LIMIT    4
#define SOCKET_PROTOCOL         SOCK_STREAM
#define MESSAGE_SIZE            128
#define PING_TIME               100000 // time for reply (in microseconds)
#define INET                    2137
#define LOCAL                   42

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>
#include <poll.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>


int error(char * msg){
    perror(msg);
    exit(-1);
}

typedef struct game_t{
    int second_player;
    int game_state[10];
    char type;
    short code;
} game_t;

typedef struct clients_t{
    int fd[SERVER_CLIENTS_LIMIT];
    string name[SERVER_CLIENTS_LIMIT];
    game_t games[SERVER_CLIENTS_LIMIT];
    int count;
    pthread_mutex_t mutex_client;
    pthread_mutex_t mutex_game;
} clients_t;

int send_message(int socket_fd, const char message[MESSAGE_SIZE]) {
    return write(socket_fd, message, MESSAGE_SIZE);
}

int receive_message(int socket_fd, char message[MESSAGE_SIZE]) {
    return read(socket_fd, message, MESSAGE_SIZE);
}

#endif
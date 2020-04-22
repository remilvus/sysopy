#ifndef PREPROC_H
#define PREPROC_H

#define or ||
#define and &&
#define true 1
#define false 0
#define not !
#define new(a,b) (calloc(a, sizeof(b)))
#define for_i_up_to(x) for(int i=0; i<x; i++)
#define for_j_up_to(x) for(int j=0; j<x; j++)
#define check_file(x) if(x==NULL){printf("failed to open file\n"); exit(-1);}
#define c_size sizeof(char)
#define min(a, b) a > b ? b : a
#define max(a, b) a > b ? a : b
#define string char*
#define forewer while(1)
#define safe_call(X) if (X < 0) error("operation failed");
#define not_null(X) if (X == NULL) error("null");
#define print printf
// #define _POSIX_C_SOURCE 1
// #define _XOPEN_SOURCE 700
// #define PROJECT_ID 12345
#define NAME_SIZE 30
#define MSG_SIZE 1024
#define MAX_MESSAGES 6
#define MAX_CLIENTS 10
#define STOP_SERVER (MAX_MSG_PRIORITY - 1)
#define STOP (MAX_MSG_PRIORITY - 2)
#define DISCONNECT (MAX_MSG_PRIORITY - 3)
#define LIST (MAX_MSG_PRIORITY - 4)
#define CONNECT (MAX_MSG_PRIORITY - 5)
#define INIT (MAX_MSG_PRIORITY - 6)
#define MESSAGE (MAX_MSG_PRIORITY - 7)
#define MAX_MSG_PRIORITY 10
#define DEBUG 0
#define LOG if(DEBUG)printf
#define SERVER_NAME "/SERVER_1234567893125"

// #define BUF_SIZE sizeof(MsgBuf) - sizeof(long)

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h> 
#include <errno.h> 
#include <math.h>
#include <signal.h>
#include <mqueue.h>
#include <sys/wait.h>
#include <sys/types.h>
// #include <sys/ipc.h>
// #include <sys/msg.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>

// typedef struct MsgBuf{
//     long mtype;
//     char mtext[MSG_SIZE];
//     int id;
//     key_t key;
// } MsgBuf;


int error(char * msg){
    perror(msg);
    exit(-1);
}

#endif

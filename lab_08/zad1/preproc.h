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
#define DEBUG false
#define LOG if(DEBUG)printf
#define BUF_SIZE sizeof(MsgBuf) - sizeof(long)

#define SIGN 0
#define BLOCK 1
#define INTERLEAVED 2
#define image_array int**

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/sem.h>
#include <errno.h> 
#include <sys/types.h>
#include <sys/ipc.h>
#include <unistd.h> 
#include <sys/wait.h>
#include <signal.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdio.h>


int error(char * msg){
    perror(msg);
    exit(-1);
}

typedef struct worker_info{
    int id;
    int workers;
}worker_info;


typedef union semun {
               int              val;    /* Value for SETVAL */
               struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
               unsigned short  *array;  /* Array for GETALL, SETALL */
               struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                           (Linux-specific) */
           }semum;



#endif
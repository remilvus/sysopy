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
#define FRST_NUM 5
#define SCND_NUM 2
#define THRD_NUM 2
#define PROJECT_ID 12345
#define TOTAL_ORDERS 30
#define MEM_SIZE 10

// for semaphore managment
#define FRST_WORKING 0
#define SCND_WORKING 1
#define ORDERS_LEFT 2
#define FREE 4 // for unused idx in shared memory
#define ORDERS_START 6
#define PACKAGES_START 8
#define SEM_SIZE 10
#define THRD_WORKING 9
#define RAND_CEIL 100
#define MEM_NAME "/MEEEEMORRRY"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h> 
#include <sys/types.h>
#include <unistd.h> 
#include <sys/wait.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>


int error(char * msg){
    perror(msg);
    exit(-1);
}

typedef union semun {
               int              val;    /* Value for SETVAL */
               struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
               unsigned short  *array;  /* Array for GETALL, SETALL */
               struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                           (Linux-specific) */
           }semum;


#endif
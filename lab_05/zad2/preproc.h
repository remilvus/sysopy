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
#define _POSIX_C_SOURCE 1
#define _XOPEN_SOURCE 700
#define MAX_COMMANDS 10
#define MAX_ARGS 20

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h> 
#include <errno.h> 
#include <sys/wait.h>
#include <math.h>
#include <signal.h>

#endif
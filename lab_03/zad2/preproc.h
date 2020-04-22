#ifndef PREPROC_H
#define PREPROC_H

#define or ||
#define and &&
#define true 1
#define false 0
#define not !
#define MIN_VALUE -100
#define MAX_VALUE 100
#define MIN_SIZE 1000
#define MAX_SIZE 1000
#define COL_WIDTH ((int)ceil(log10((double)(MAX_SIZE*MAX_VALUE*MAX_VALUE))) + 15)
#define new(a,b) (calloc(a, sizeof(b)))
#define for_i_up_to(x) for(int i=0; i<x; i++)
#define for_j_up_to(x) for(int j=0; j<x; j++)
#define c_size sizeof(char)
#define min(a, b) a > b ? b : a
#define max(a, b) a > b ? a : b
#define string char*

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h> 
#include <errno.h>
#include <sys/times.h>
#include <sys/types.h> 
#include <sys/wait.h>
#include <math.h>
#include <sys/file.h>
#include <sys/stat.h>

#endif
#ifndef PREPROC_H
#define PREPROC_H

#define or ||
#define and &&
#define true 1
#define false 0
#define not !
#define new(a,b) (calloc(a, sizeof(b)))
#define _XOPEN_SOURCE 500

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/times.h>
#include <unistd.h>
#include <fcntl.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <ftw.h>

#endif
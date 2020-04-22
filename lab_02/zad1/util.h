#ifndef UTIL_H
#define UTIL_H

#include "preproc.h"
double timeDiff(clock_t start, clock_t end);
void saveTime(FILE* reportFile, clock_t start, clock_t end, struct tms* t_start, struct tms* t_end);
void myReadLine(int fd, char* buf, int offset, int LENN);
void fmyReadLine(FILE* fd, char* buf, int offset, int LENN);
void writeLine(int fd, char* buf, int offset, int LENN);
void fwriteLine(FILE* fd, char* buf, int offset, int LENN);
int is_lex_first(char* str1, char* str2);
void generateRandFile(char* filename, int SIZE, int LENN);
void copy(char* f1, char* f2, int SIZE, int LENN);
void fcopy(char* f1, char* f2, int SIZE, int LENN);

#endif
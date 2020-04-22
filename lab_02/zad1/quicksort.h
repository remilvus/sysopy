#ifndef QUICKSORT_H
#define QUICKSORT_H

#include "preproc.h"

void sort(char* filename, int LENN, int SIZE);
void fsort(char* filename, int LENN, int SIZE);
int partition (int fd, int low, int high, int LENN);
int fpartition(FILE *fd, int low, int high, int LENN);
void quickSort(int fd, int low, int high, int LENN);
void fquickSort(FILE* fd, int low, int high, int LENN);

#endif
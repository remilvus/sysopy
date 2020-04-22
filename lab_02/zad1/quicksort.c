#include "quicksort.h"

void sort(char* filename, int LENN, int SIZE){
    int fd = open(filename, O_RDWR);
    quickSort(fd, 0, LENN * SIZE - LENN, LENN);
    close(fd);
}

void fsort(char* filename, int LENN, int SIZE){
    FILE* fd = fopen(filename, "r+");
    if(fd==NULL){
        printf("failed to open file");
        exit(-1);
    }
    fquickSort(fd, 0, LENN * SIZE - LENN, LENN);
    fclose(fd);
}


int partition (int fd, int low, int high, int LENN)
{
    if(low >= high) return low;
    char* line = new(LENN, char);
    char* pivot = new(LENN, char);

    myReadLine(fd, pivot, low, LENN);
    if (low + LENN == high){
        myReadLine(fd, line, high, LENN);
        if(is_lex_first(line, pivot)){
            writeLine(fd, pivot, high, LENN);
            writeLine(fd, line, low, LENN);
            free(line);
            free(pivot);
            return high;
        }
    }
    int left = low + LENN; 
    int right = high;
    while(true){
        myReadLine(fd, line, left, LENN);
        while(is_lex_first(line, pivot) and left <= right){
            left += LENN;
            myReadLine(fd, line, left, LENN);
        }

        myReadLine(fd, line, right, LENN);
        while(not is_lex_first(line, pivot) and left <= right){
            right -= LENN;
            myReadLine(fd, line, right, LENN);
        }

        if(left >= right) break;
        myReadLine(fd, pivot, left, LENN);
        writeLine(fd, pivot, right, LENN);
        writeLine(fd, line, left, LENN);
        myReadLine(fd, pivot, low, LENN);
    }
  //  if(left == right) right -= LENN;
    myReadLine(fd, line, right, LENN);
    writeLine(fd, pivot, right, LENN);
    writeLine(fd, line, low, LENN);
    free(line);
    free(pivot);
    return right;
}

int fpartition(FILE *fd, int low, int high, int LENN)
{
    if (low >= high)
        return low;
    char *line = new (LENN, char);
    char *pivot = new (LENN, char);

    fmyReadLine(fd, pivot, low, LENN);
    if (low + LENN == high)
    {
        fmyReadLine(fd, line, high, LENN);
        if (is_lex_first(line, pivot))
        {
            fwriteLine(fd, pivot, high, LENN);
            fwriteLine(fd, line, low, LENN);
            free(line);
            free(pivot);
            return high;
        }
    }
    int left = low + LENN;
    int right = high;
    while (true)
    {
        fmyReadLine(fd, line, left, LENN);
        while (is_lex_first(line, pivot) and left <= right)
        {
            left += LENN;
            fmyReadLine(fd, line, left, LENN);
        }

        fmyReadLine(fd, line, right, LENN);
        while (not is_lex_first(line, pivot) and left <= right)
        {
            right -= LENN;
            fmyReadLine(fd, line, right, LENN);
        }

        if (left >= right)
            break;
        fmyReadLine(fd, pivot, left, LENN);
        fwriteLine(fd, pivot, right, LENN);
        fwriteLine(fd, line, left, LENN);
        fmyReadLine(fd, pivot, low, LENN);
    }
    fmyReadLine(fd, line, right, LENN);
    fwriteLine(fd, pivot, right, LENN);
    fwriteLine(fd, line, low, LENN);
    free(line);
    free(pivot);
    return right;
}

void quickSort(int fd, int low, int high, int LENN){
    if (low < high)
    {
        int pi = partition(fd, low, high, LENN);

        quickSort(fd, low, pi - LENN, LENN);  // Before pi
        quickSort(fd, pi + LENN, high, LENN); // After pi
    }
}

void fquickSort(FILE* fd, int low, int high, int LENN){
    if (low < high)
    {
        int pi = fpartition(fd, low, high, LENN);

        fquickSort(fd, low, pi - LENN, LENN);  // Before pi
        fquickSort(fd, pi + LENN, high, LENN); // After pi
    }
}
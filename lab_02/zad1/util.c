#include "util.h"

double timeDiff(clock_t start, clock_t end){
    return ((double)(end - start) / sysconf(_SC_CLK_TCK));
}

void saveTime(FILE* reportFile, clock_t start, clock_t end, struct tms* t_start, struct tms* t_end){
    fprintf(reportFile, "\tREAL_TIME: %.50f\n", timeDiff(start, end));
    fprintf(reportFile, "\tUSER_TIME: %.50f\n", timeDiff(t_start->tms_utime, t_end->tms_utime));
    fprintf(reportFile, "\tSYSTEM_TIME: %.50f\n\n", timeDiff(t_start->tms_stime, t_end->tms_stime));
}

void myReadLine(int fd, char* buf, int offset, int LENN){
    lseek(fd, offset, SEEK_SET);
    read(fd, buf, LENN);
    buf[LENN - 1]=0;
}

void fmyReadLine(FILE* fd, char* buf, int offset, int LENN){
    fseek(fd, offset, 0);
    fread(buf, sizeof(char), LENN, fd);
    buf[LENN - 1]=0;
}

void writeLine(int fd, char* buf, int offset, int LENN){
    lseek(fd, offset, SEEK_SET);
    write(fd, buf, LENN - 1);
}

void fwriteLine(FILE* fd, char* buf, int offset, int LENN){
    fseek(fd, offset, 0);
    fwrite(buf, sizeof(char), LENN - 1, fd);
}

int is_lex_first(char* str1, char* str2){
    if (strcmp(str1, str2) < 0) return true;
    return false;
}


void generateRandFile(char* filename, int SIZE, int LENN){
    time_t t;
    srand((unsigned) time(&t));

    FILE* f;
    f = fopen(filename, "w+");
    if(f==NULL){
        printf("Failed to open file\n");
        exit(-1);
    }
    char* line = new(LENN + 1, char);
    for(int i=0; i<SIZE; i++){
        for(int j=0; j<LENN-1; j++){
            line[j]= rand() % 26 + 97;
        }
        line[LENN]=0;
        fprintf(f,"%s\n", line);
    }
    free(line);
    fclose(f);    
}

void copy(char* f1, char* f2, int SIZE, int LENN){
    int fd1 = open(f1, O_RDONLY);
    int fd2 = open(f2, O_WRONLY | O_CREAT, S_IRWXU);
    char* buf = new(LENN, char);
    for(int i=0; i<SIZE; i++){
        int r = read(fd1, buf, LENN);
        if(r<=0) {
            printf("Not enough records to copy");
            exit(-1);
        };
        write(fd2, buf, LENN);
    }
    free(buf);
    close(fd1);
    close(fd2);
}

void fcopy(char* f1, char* f2, int SIZE, int LENN){
    FILE* fd1 = fopen(f1, "r");
    FILE* fd2 = fopen(f2, "w+");
    if(fd1 == NULL or fd2==NULL){
        printf("Failed to open the file\n");
        exit(-1);
    }
    char* buf = new(LENN, char);
    for(int i=0; i<SIZE; i++){
        int r = fread(buf, sizeof(char), LENN, fd1);
        if(r<=0) {
            printf("Not enough records to copy");
            exit(-1);
        };
        fwrite(buf, sizeof(char), LENN, fd2);
    }
    free(buf);
    fclose(fd1);
    fclose(fd2);
}
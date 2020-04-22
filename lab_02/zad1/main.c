#include "preproc.h"

int main(int argc, char **argv){
    int job_idx = 1;
    if(job_idx > argc) exit(0);
    int LEN = 0;
    int LENN = 0;
    int SIZE = 0;
    char* filename;
    FILE* reportf = fopen("report.txt", "a");
    struct tms* tms_start = calloc(1, sizeof(struct tms));
    clock_t time_start;
    struct tms* tms_end = calloc(1, sizeof(struct tms));
    clock_t time_end;

    while(job_idx < argc){
        time_start = times(tms_start);

        if(strcmp(argv[job_idx], "generate")==0 and job_idx + 3 < argc){
            printf("generate\n");
            filename = strdup(argv[job_idx + 1]);
            SIZE = atoi(argv[job_idx + 2]);
            LEN = atoi(argv[job_idx + 3]);
            if(SIZE == 0 or LEN==0 or strcmp(filename, ".")==0){
                printf("Bad argument");
                return -1;
            }
            fprintf(reportf, "generate | record num: %d | record len: %d\n", SIZE, LEN);
            generateRandFile(filename, SIZE, LEN + 1);
            job_idx+=4;
        } else if (strcmp(argv[job_idx], "sort")==0  and job_idx + 3 < argc)
        {
            printf("sys-sort\n");
            fprintf(reportf, "sys-sort\n");
            filename = strdup(argv[job_idx + 1]);
            SIZE = atoi(argv[job_idx + 2]);
            LEN = atoi(argv[job_idx + 3]);
            if(SIZE == 0 or LEN==0 or strcmp(filename, ".")==0){
                printf("Bad argument");
                return -1;
            }
            LENN = LEN+1;
            sort(filename, LENN, SIZE);
            job_idx+=4;
        } else if (strcmp(argv[job_idx], "libsort")==0  and job_idx + 3 < argc)
        {
            printf("lib-sort\n");
            fprintf(reportf, "lib-sort\n");
            filename = strdup(argv[job_idx + 1]);
            SIZE = atoi(argv[job_idx + 2]);
            LEN = atoi(argv[job_idx + 3]);
            if(SIZE == 0 or LEN==0 or strcmp(filename, ".")==0){
                printf("Bad argument");
                return -1;
            }
            LENN = LEN+1;
            fsort(filename, LENN, SIZE);
            job_idx+=4;
        } else if(strcmp(argv[job_idx], "copy")==0 and job_idx + 4 < argc){
            printf("lib-copy\n");
            fprintf(reportf, "lib-copy\n");
            SIZE = atoi(argv[job_idx + 3]);
            LEN = atoi(argv[job_idx + 4]);
            if(SIZE == 0 or LEN==0){
                printf("Bad argument");
            }
            LENN = LEN + 1;
            fcopy(argv[job_idx+1], argv[job_idx+2], SIZE, LENN);
            job_idx+=5;
        } else if(strcmp(argv[job_idx], "syscopy")==0 and job_idx + 4 < argc){
            printf("sys-copy\n");
            fprintf(reportf, "sys-copy\n");
            SIZE = atoi(argv[job_idx + 3]);
            LEN = atoi(argv[job_idx + 4]);
            if(SIZE == 0 or LEN==0){
                printf("Bad argument");
            }
            LENN = LEN + 1;
            copy(argv[job_idx+1], argv[job_idx+2], SIZE, LENN);
            job_idx+=5;
        }else {
            fprintf(reportf, "Bad argument\n");
            printf("Bad argument (%s)\n", argv[job_idx]);
            job_idx++;
        }

        time_end = times(tms_end);
        saveTime(reportf, time_start, time_end, tms_start, tms_end);
    }
    fclose(reportf);
}
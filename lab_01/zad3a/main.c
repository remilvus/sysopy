#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/times.h>
#include "mylib.h"
#define DEBUG 0

double timeDiff(clock_t start, clock_t end){
    return ((double)(end - start) / sysconf(_SC_CLK_TCK));
}

void saveTime(FILE* reportFile, clock_t start, clock_t end, struct tms* t_start, struct tms* t_end){
    printf("\tREAL_TIME: %.50f\n", timeDiff(start, end));
    printf("\tUSER_TIME: %.50f\n", timeDiff(t_start->tms_utime, t_end->tms_utime));
    printf("\tSYSTEM_TIME: %.50f\n\n", timeDiff(t_start->tms_stime, t_end->tms_stime));

    fprintf(reportFile, "\tREAL_TIME: %.50f\n", timeDiff(start, end));
    fprintf(reportFile, "\tUSER_TIME: %.50f\n", timeDiff(t_start->tms_utime, t_end->tms_utime));
    fprintf(reportFile, "\tSYSTEM_TIME: %.50f\n\n", timeDiff(t_start->tms_stime, t_end->tms_stime));
}

int count_pairs(char **argv, int first, int size){
    int num = 0;
    while(first + num < size){
        char* d = strchr(argv[first + num], ':');
        if (d!=NULL) num++;
        else break;
    }
    return num;
}

int is_idx(char* str, int num){
    return num!=0 or (isdigit(str[0]) and strlen(str)==1); // idx!=0 or idx==0
}

int main(int argc, char **argv){
    int job_idx = 1;
    int size = 0;
    FILE* report_file = fopen("./raport2.txt", "w");
    struct tms* tms_start = calloc(1, sizeof(struct tms));
    clock_t time_start;
    struct tms* tms_end = calloc(1, sizeof(struct tms));
    clock_t time_end;

    operationBlock** main_array = NULL;
    while(job_idx < argc){
        time_start = times(tms_start);

        if(strcmp(argv[job_idx], "create_table") == 0){
            printf("Job detected: create table (size %s)\n", argv[job_idx+1]);
            fprintf(report_file, "Job detected: create table (size %s)\n", argv[job_idx+1]);
            if(main_array!=NULL){
                printf("Array already exists");
                job_idx += 2;
                continue;
            }
            size = atoi(argv[job_idx+1]);
            if(size==0){
                printf("Bad argument");
                return -1;
            }
            main_array = createBlockArray(size);

            job_idx += 2;
        } else if (strcmp(argv[job_idx], "compare_pairs") == 0)
        {
            printf("Job detected: compare_pairs\n");
            fprintf(report_file, "Job detected: compare_pairs\n");
            if(main_array==NULL){
                printf("Array for output was not created");
                return -1;
            }
            int pair_num = count_pairs(argv, job_idx + 1, argc);
            filePair* pairs = makePairs(&argv[job_idx + 1], pair_num);
            for(int i=0; i<pair_num; i++){
                compare(pairs[i]);
                int b_idx = createBlock(main_array, size);
                printf("%s  |  %s  | block: %d\n", pairs[i].first, pairs[i].second, b_idx);
                fprintf(report_file, "%s  |  %s  | block: %d\n", pairs[i].first, pairs[i].second, b_idx);
            }
            job_idx += 1 + pair_num;
        }else if (strcmp(argv[job_idx], "remove_block") == 0)
        {
            printf("Job detected: remove_block (idx: %s)\n", argv[job_idx + 1]);
            fprintf(report_file, "Job detected: remove_block (idx: %s)\n", argv[job_idx + 1]);
            int block = atoi(argv[job_idx+1]);
            if(!is_idx(argv[job_idx+1], block) or block>=size){
                printf("Bad argument");
                return -1;
            }
            deleteBlock(main_array, block, size);
            job_idx += 2;
        }else if (strcmp(argv[job_idx], "remove_operation") == 0)
        {
            printf("Job detected: remove_operation (block idx: %s | operation idx: %s)\n",
                                                                argv[job_idx+1], argv[job_idx+2]);
            fprintf(report_file, "Job detected: remove_operation (block idx: %s | operation idx: %s)\n",
                                                                argv[job_idx+1], argv[job_idx+2]);
            int block = atoi(argv[job_idx+1]);
            int operation = atoi(argv[job_idx+2]);
            if(!is_idx(argv[job_idx+1], block) or !is_idx(argv[job_idx+2], operation) or block>=size){
                printf("Bad argument");
                return -1;
            }
            int res = deleteOperation(main_array, block, operation, size);
            if(res == -1) {
                printf("Bad Argument");
                return -1;
            }
            job_idx += 3;
        }else{
            printf("Bad argument (%s is not a job)", argv[job_idx]);
            job_idx += 1;
        }

        time_end = times(tms_end);
        saveTime(report_file, time_start, time_end, tms_start, tms_end);
    }

    for(int i=0; i<size; i++){
        if(main_array[i] == NULL) continue;
       // for(int j=0; j<main_array[i]->len; j++){
       //     printf("operation:\n%s\n", main_array[i]->operations[j]);
       // }
      //  printf("deleting block number %d\n", i);
        deleteBlock(main_array, i, size);
    }

    fclose(report_file);
  //  free(main_array);
}


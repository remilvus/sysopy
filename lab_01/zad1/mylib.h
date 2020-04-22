#define or ||
#define and &&

#include <stdlib.h>
//#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h> //is digit
#include <string.h>


typedef struct operationBlock_t operationBlock;
struct operationBlock_t {
    int len;
    char** operations;
};

typedef struct filePair_t filePair;
struct filePair_t{
    char* first;
    char* second;
};

void compare(filePair);
int createBlock(operationBlock**, int);
operationBlock** createBlockArray(int);
int find_empty(operationBlock**, int);
filePair* makePairs(char**, int);
int deleteBlock(operationBlock** block_array, int idx, int size);
int deleteOperation(operationBlock** block_array, int b_idx, int op_idx, int size);
int countOperations(FILE* file);
int getOperationLen(FILE* file, long block_start);
long saveOperation(FILE* file, operationBlock* block, int i, int len);
#include "mylib.h"

void compare(filePair pair){
    int buf_size = strlen(pair.first) + strlen(pair.second);
    buf_size += strlen("diff  > tmp.txt");
    char* buffer = calloc(buf_size + 1, sizeof(char));
    sprintf(buffer, "diff %s %s > tmp.txt", pair.first, pair.second); // create system command with names of files
    int res = system(buffer);
    if (res == -1) exit(-1);
}

int createBlock(operationBlock** block_array, int size){
    int idx = find_empty(block_array, size);
    if(idx==-1) return -1;

    // create block
    operationBlock* block;
    block = calloc(1, sizeof(operationBlock));
    block_array[idx] = block;

    // count operations
    FILE* file = fopen("tmp.txt", "r");
    int operations_num = countOperations(file);

    // reserve memory for pointers to operations
    if(operations_num!=0) block->operations = calloc(operations_num, sizeof(char*));
    block->len = operations_num;

    // save all operations to block
    long block_start = ftell(file);
    for (int i=0; i<operations_num; i++){
        int op_len = getOperationLen(file, block_start); // determine size of an operation
        block_start = saveOperation(file, block, i, op_len); // save operation to the block
    }
    fclose(file);
    return idx;
}

int countOperations(FILE* file){ // counts how many operations (from diff) are in a file
    long file_beg = ftell(file);
    size_t bufsize = 0;
    char *lineptr;
    int read = getline(&lineptr, &bufsize, file);
    int operations_num=0;
    while(read != -1){
        if (isdigit(lineptr[0])) ++operations_num;
        read = getline(&lineptr, &bufsize, file);
    }
    fseek(file, file_beg, SEEK_SET);
    return operations_num;
}

int getOperationLen(FILE* file, long block_start){
    int len = 0;
    char *lineptr;
    size_t bufsize = 0;
    int read = 0;
    read = getline(&lineptr, &bufsize, file);
    do {
        len += read;
        read = getline(&lineptr, &bufsize, file);
    } while(read != -1 or isdigit(lineptr[0]));
    fseek(file, block_start, SEEK_SET);
    return len;
}

long saveOperation(FILE* file, operationBlock* block, int op_idx, int len){
    char *lineptr;
    size_t bufsize = 0;
    block->operations[op_idx] = calloc(len + 1 + 8, sizeof(char)); // len + null + ?long?
    int read_sum = 0;
    int read = getline(&lineptr, &bufsize, file);
    long block_start;
    do {
        strcpy(&block->operations[op_idx][read_sum], lineptr);
        read_sum += read;
        block_start = ftell(file);
        read = getline(&lineptr, &bufsize, file);
    } while(read != -1 and !isdigit(lineptr[0]));
    return block_start;
}

operationBlock** createBlockArray(int size){
    operationBlock** array= calloc(size, sizeof(operationBlock*));
    return array;
};

int find_empty(operationBlock** block_array, int size){
    for(int i=0; i<size; i++){
        if(block_array[i]==NULL) return i;
    }
    return -1;
}

filePair* makePairs(char** names, int num){
    filePair* pairs = calloc(num, sizeof(filePair));
    for(int i=0; i<num; i++){
        char*q = strchr(names[i], ':');
        int delimiter = (int)(q - names[i]);
        pairs[i].first = calloc(delimiter + 1, sizeof(char));
        pairs[i].second = calloc(strlen(names[i]) - delimiter + 1, sizeof(char));
        strncpy(pairs[i].first, names[i], delimiter);
        strcpy(pairs[i].second, &names[i][delimiter + 1]);
    }
    return pairs;
}

int deleteBlock(operationBlock** block_array, int idx, int size){
    if(idx >= size) return -1;
    if(block_array[idx]==NULL) return 0;
    if(block_array[idx]->len != 0){
        for(int i=0;i<block_array[idx]->len; i++){
            if(block_array[idx]->operations[i] != NULL) free(block_array[idx]->operations[i]);
        }
        free(block_array[idx]->operations);
    }
    free(block_array[idx]);
    block_array[idx]=NULL;
    return 0;
}

int deleteOperation(operationBlock** block_array, int b_idx, int op_idx, int size){
    if(b_idx >= size or block_array[b_idx]==NULL or block_array[b_idx]->len <= op_idx) return -1;
    free(block_array[b_idx]->operations[op_idx]);
    int last = block_array[b_idx]->len - 1;
    block_array[b_idx]->operations[op_idx] = block_array[b_idx]->operations[last];
    block_array[b_idx]->len -= 1;
    return 0;
}

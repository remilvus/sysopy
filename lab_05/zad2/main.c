#include "preproc.h"
#define USE_POPEN 0



int main(int argc, char** argv) {
    size_t size = 128;
    string line = new(size, char);
    
    FILE* sort_input = popen("sort", "w");
    FILE* unsorted_file = fopen(argv[1], "r");
    check_file(unsorted_file);
    check_file(sort_input);
    int res;
    res = getline(&line, &size, unsorted_file);
   //  printf("%s\n", line);
    while(res!=-1){
        fwrite(line, c_size, res, sort_input);
        res = getline(&line, &size, unsorted_file);
     //   printf("%s\n", line);
    }

    pclose(sort_input);
    fclose(unsorted_file);
    free(line);
    return 0;
}
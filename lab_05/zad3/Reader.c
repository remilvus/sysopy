#include "preproc.h"


int main(int argc, string* argv){
    FILE* input = fopen(argv[1], "r");
    FILE* output = fopen(argv[2], "w+"); 
    int N = atoi(argv[3]);
    check_file(output);
    check_file(input);
    assert_natural(N);

    string buffer = new(N, char);

    while(true){
        int res = fread(buffer, c_size, N, input);
        if (res==0) break;
       // printf("read:>%s<%d\n", buffer, res);

        fwrite(buffer, c_size, res, output);
        fflush(output);
        memset(buffer, c_size, N);
    }

    fclose(output);
}
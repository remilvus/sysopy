#include "preproc.h"


int main(int argc, string* argv){
    FILE* output = fopen(argv[1], "r+");
    FILE* input = fopen(argv[2], "r"); 
    int N = atoi(argv[3]);
    check_file(output);
    check_file(input);
    assert_natural(N);
    srand(time(NULL));

    string buffer = new(N, char);
    string line = new(N + 50, char);
    printf("writer %s starting\n", argv[2]);
    while(true){
        int res_buf = fread(buffer, c_size, N, input);
        if (res_buf==0) break;

     //   printf("%s\n", buffer);
     //   fclose(output);
        sleep(rand()%3+1);
        output = fopen(argv[1], "r+");
        int res = sprintf(line, "#%d#%s",getpid(), buffer);
      //  printf(">%s<write<%d>\n",line, res - N + res_buf);
        line[res - N + res_buf] = '\n';
        fwrite(line, c_size, res - N + res_buf + 1, output);
        fflush(output);
   //     perror("linewrr");
        memset(buffer, c_size, N);
        memset(line, c_size, N + 50);
    }
    printf("writer %s ended\n", argv[2]);
}
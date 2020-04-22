#include "preproc.h"


int main(int argc, string* argv){
    system("rm -f fifo");
    system("mkfifo fifo");
    int pid = fork();
    if (pid==0){
        system("./reader fifo out.txt 6");
        return 0;
    }
    pid = fork();
    if (pid==0){
        system("./writer fifo in0.txt 7");
        return 0;
    }
    pid = fork();
    if (pid==0){
        system("./writer fifo in1.txt 6");
        return 0;
    }
    pid = fork();
    if (pid==0){
        system("./writer fifo in2.txt 11");
        return 0;
    }
    pid = fork();
    if (pid==0){
        system("./writer fifo in3.txt 4");
        return 0;
    }
    pid = fork();
    if (pid==0){
        system("./writer fifo in4.txt 22");
        return 0;
    }
}
#include "preproc.h"

#define USE_EXEC 1

void communicate(int signum){
    printf("Otrzymano SIGUSR1\n");
}

void handle_arg(string arg){
     struct sigaction sig;
    if(strcmp(arg, "ignore")==0){
                printf("ignore\n");
                sig.sa_handler = SIG_IGN;
                sigaction(SIGUSR1, &sig, NULL);
                raise(SIGUSR1);
            } else if(strcmp(arg, "handler")==0) {
                printf("handler\n");
                sig.sa_handler = communicate;
                sigaction(SIGUSR1, &sig, NULL);
                raise(SIGUSR1);
            } else if(strcmp(arg, "mask")==0) {
                printf("mask\n");
                sigset_t set; sigaddset(&set, SIGUSR1);
                sigprocmask(SIG_SETMASK, &set, NULL);
                raise(SIGUSR1);
                // sigemptyset(&set);
                // sigpending(&set);
                // if(sigismember(&set, SIGUSR1)){
                //     printf("Sygnał SIGUSR1 oczekuje\n");
                // } else {
                //     printf("Sygnał SIGUSR1 nie oczekuje\n");
                // }
            } else if(strcmp(arg, "pending")==0) {
                printf("pending\n");
                sigset_t set;
                sigpending(&set);
                if(sigismember(&set, SIGUSR1)){
                    printf("Sygnał SIGUSR1 oczekuje\n");
                } else {
                    printf("Sygnał SIGUSR1 nie oczekuje\n");
                }
            } else {
                printf("Bad argument\n");
            }
}

int main(int argc, string* argv){

    if(not USE_EXEC){
        // using fork
        if(strcmp(argv[1], "pending")==0){
            handle_arg("mask");
        }else{
            handle_arg(argv[1]);
        }
        printf("fork\n");
        pid_t pid = fork();
        if (pid==0) {
             if(strcmp(argv[1], "pending")==0){
                handle_arg(argv[1]);
             }else{
                raise(SIGUSR1);
             }
        }
    } else {
        //using exec
        if(strcmp(argv[1], "pending")==0){
            handle_arg("mask");
        }else if(strcmp(argv[1], "pending2")==0){
            handle_arg("pending");
        }else if(strcmp(argv[1], "raise")==0){
            raise(SIGUSR1);
        } else {
            handle_arg(argv[1]);
        }
        if(strcmp(argv[1], "pending")==0){
            printf("exec\n");
            execl("./main", "main", "pending2", NULL);
            return 0;
        } 
        if (strcmp(argv[1], "raise")!=0 and strcmp(argv[1], "pending2")!=0){
            printf("exec\n");
            execl("./main", "main", "raise", NULL);
        }

    }
}
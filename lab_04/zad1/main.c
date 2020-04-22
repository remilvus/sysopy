#include "preproc.h"

void stop(int signum) {return;};

void intReaction(int signum){
    printf("\nOdebrano sygnał SIGINT\n");
    exit(0);
}

void stopReaction(int signum){
    struct sigaction sig;
    sig.sa_handler = stop;
    sig.sa_flags = SA_NODEFER;
    sigaction(SIGTSTP, &sig, NULL);
     signal(SIGINT, SIG_DFL);
    printf("\nOczekuję na CTRL+Z - kontynuacja albo CTR+C - zakończenie programu\n");
    pause();
    sig.sa_handler = stopReaction;
    sigaction(SIGTSTP, &sig, NULL);
    signal(SIGINT, intReaction);
}

int main(int argc, string* argv){
    struct sigaction sig;
    sig.sa_handler = stopReaction;
    sig.sa_flags = SA_NODEFER;
    sigaction(SIGTSTP, &sig, NULL);
    signal(SIGINT, intReaction);
    while(true){
        system("ls");
        sleep(1);
    }
}
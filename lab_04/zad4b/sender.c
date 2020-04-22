#include "preproc.h"

int SIG_COUNT = 0;
int SIG_TOTAL = 0;
int REACT_TO = SIGUSR1;
int COUNT_SIG;
int END_SIG;
union sigval empty;
string mode;

void set_block(){
    struct sigaction block;
    sigfillset(&block.sa_mask);
    if(strcmp(mode, "sigrt")==0){sigdelset(&block.sa_mask, COUNT_SIG); sigdelset(&block.sa_mask, END_SIG);}
    else {sigdelset(&block.sa_mask, SIGUSR1); sigdelset(&block.sa_mask, SIGUSR1);}
    sigaction(SIG_BLOCK, &block, NULL);
}

void end(int sig, siginfo_t *info, void *ucontext){
    printf("Sender received %d signals | should've received: %d\n", SIG_COUNT, SIG_TOTAL);
    exit(0);
}

void count(int sig, siginfo_t *info, void *ucontext){
    SIG_COUNT++;
}

int main(int argc, string* argv){
    COUNT_SIG = SIGRTMIN;
    END_SIG = SIGRTMIN+1;
    pid_t catcher = (pid_t)atoi(argv[1]);
    SIG_TOTAL = atoi(argv[2]);
    mode = "sigrt";//argv[3];

    set_block();
    sigset_t suspend_mask;
    sigfillset(&suspend_mask);
    if(strcmp(mode, "sigrt")==0){sigdelset(&suspend_mask, COUNT_SIG); sigdelset(&suspend_mask, END_SIG);}
    else {sigdelset(&suspend_mask, SIGUSR1); sigdelset(&suspend_mask, SIGUSR2); }

    struct sigaction sig;
    sig.sa_flags = SA_SIGINFO;
    sig.sa_sigaction = count;
    if(strcmp(mode, "sigrt")==0) REACT_TO=COUNT_SIG;
    sigaction(REACT_TO, &sig, NULL);

    struct sigaction sig2;
    sig2.sa_flags = SA_SIGINFO;
    sig2.sa_sigaction = end;
    if(strcmp(mode, "sigrt")==0) sigaction(END_SIG, &sig2, NULL);
    else sigaction(SIGUSR2, &sig2, NULL);

    for_i_up_to(SIG_TOTAL){

        kill(catcher, COUNT_SIG);

        sigsuspend(&suspend_mask);

        sigaction(REACT_TO, &sig, NULL);
    }

    kill(catcher, END_SIG);


    while(true){

    }

}

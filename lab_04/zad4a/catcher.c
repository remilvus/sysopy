#include "preproc.h"

int SIG_COUNT = 0;
int COUNT_SIG;
int END_SIG;
int REACT_TO = SIGUSR1;
string mode;
union sigval empty;

void counter(int signo, siginfo_t *info, void *ucontext){
    SIG_COUNT ++;
    struct sigaction sig;
    sig.sa_flags = SA_SIGINFO;
    sig.sa_sigaction = counter;
    sigaction(REACT_TO, &sig, NULL);
}

void resend(int sig, siginfo_t *info, void *ucontext){
    pid_t sender = info->si_pid;
    for_i_up_to(SIG_COUNT){
        if(strcmp(mode, "kill")==0) kill(sender, SIGUSR1);
        if(strcmp(mode, "sigqueue")==0){ 
            empty.sival_int = i;
            sigqueue(sender, SIGUSR1, empty);
        }
        if(strcmp(mode, "sigrt")==0) kill(sender, COUNT_SIG);
    }
    empty.sival_int = SIG_COUNT;
    printf("catcher received %d signals\n", SIG_COUNT);
    if(strcmp(mode, "kill")==0) kill(sender, SIGUSR2);
    if(strcmp(mode, "sigqueue")==0) sigqueue(sender, SIGUSR2, empty);
    if(strcmp(mode, "sigrt")==0) kill(sender, END_SIG);
    exit(0);
}

void set_block(){
    struct sigaction block;
    sigfillset(&block.sa_mask);
    if(strcmp(mode, "sigrt")==0)
    {sigdelset(&block.sa_mask, COUNT_SIG); 
    sigdelset(&block.sa_mask, END_SIG);}
    else {sigdelset(&block.sa_mask, SIGUSR1); sigdelset(&block.sa_mask, SIGUSR2);}
    sigaction(SIG_BLOCK, &block, NULL);
}

int main(int argc, string* argv){
    COUNT_SIG = SIGRTMIN;
    END_SIG = SIGRTMIN+1;
    mode = argv[1];
    printf("catcher mode %s\n", mode);
    printf("Catcher PID: %d\n", getpid());

    set_block();

    struct sigaction sig;
    sig.sa_flags = SA_SIGINFO;
    sig.sa_sigaction = counter;
    if(strcmp(mode, "sigrt")==0) REACT_TO=COUNT_SIG;
    sigaction(REACT_TO, &sig, NULL);

    struct sigaction sig2;
    sig2.sa_flags = SA_SIGINFO;
    sig2.sa_sigaction = resend;
    if(strcmp(mode, "sigrt")==0) sigaction(END_SIG, &sig2, NULL);
    else sigaction(SIGUSR2, &sig2, NULL);

    while(true){

    }

}


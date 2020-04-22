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
    if(strcmp(mode, "sigqueue")==0) 
        printf("catcher've resent %d signals\n", info->si_value.sival_int);
    exit(0);
}


void count(int signo, siginfo_t *info, void *ucontext){
 //   if(strcmp(mode, "sigqueue")==0) 
  //      printf("sender recevied SIGUSR1 num.%d\n", info->si_value.sival_int);
    SIG_COUNT++;
    struct sigaction sig;
    sig.sa_flags = SA_SIGINFO;
    sig.sa_sigaction = count;
    sigaction(REACT_TO, &sig, NULL);
}

int main(int argc, string* argv){
    COUNT_SIG = SIGRTMIN;
    END_SIG = SIGRTMIN+1;
    pid_t catcher = (pid_t)atoi(argv[1]);
    SIG_TOTAL = atoi(argv[2]);
    mode = argv[3];

    set_block();

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
        if(strcmp(mode, "kill")==0) kill(catcher, SIGUSR1);
        if(strcmp(mode, "sigqueue")==0) sigqueue(catcher, SIGUSR1, empty);
        if(strcmp(mode, "sigrt")==0) {kill(catcher, COUNT_SIG);}
    }
    sleep(1);
    if(strcmp(mode, "kill")==0) kill(catcher, SIGUSR2);
    if(strcmp(mode, "sigqueue")==0) sigqueue(catcher, SIGUSR2, empty);
    if(strcmp(mode, "sigrt")==0) kill(catcher, END_SIG);


    while(true){

    }

}

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
void sighandler(int signo,  siginfo_t *info, void *ucontext){
    printf("received value %d\n", info->si_value.sival_int);
    exit(0);
}



int main(int argc, char* argv[]) {

    if(argc != 3){
        printf("Not a suitable number of program parameters\n");
        return 1;
    }

    struct sigaction action;
    action.sa_flags = SA_SIGINFO;
    action.sa_sigaction = &sighandler;

    //..........
    sigfillset(&action.sa_mask); // blokowanie sygnałów
    sigdelset(&action.sa_mask, SIGUSR1);

    int signo = atoi(argv[2]);
    int val = atoi(argv[1]);

    int child = fork();
    if(child == 0) {
        //zablokuj wszystkie sygnaly za wyjatkiem SIGUSR1
        //zdefiniuj obsluge SIGUSR1 w taki sposob zeby proces potomny wydrukowal
        //na konsole przekazana przez rodzica wraz z sygnalem SIGUSR1 wartosc
        sigaction(SIGUSR1, &action, NULL);
        pause(); // aby rodzić zdążył wysłać sygnał
    }
    else {
        //wyslij do procesu potomnego sygnal przekazany jako argv[2]
        //wraz z wartoscia przekazana jako argv[1]
        union sigval sigval;
        sigval.sival_int = val;
        sleep(1);
        sigqueue(child, signo, sigval);
    }
    
    
    return 0;
}

#include "preproc.h"


void handler(int sig, siginfo_t *info, void *ucontext){
    printf("otrzymany sygnał: %d\n", sig);
    printf("uid użytkownika, którego proces wysłał sygnał: %d\n", (int)info->si_uid);
    printf("pamięć związana z sygnałem; memory loc: %lld\n", (long long int)info->si_addr);
    printf("dodatkowa informacja; si_val: %d\n", info->si_value.sival_int);
    exit(0);
}

int main(int argc, string* argv){
    struct sigaction sig;
    sig.sa_flags = SA_SIGINFO;
    sig.sa_sigaction = handler;

    // first scenario
    sigaction(SIGSEGV, &sig, NULL);
 //   **(argv-c_size*10) = 1;

    // second scenario
    union sigval info;
    info.sival_int = 42;
    sigaction(SIGUSR1, &sig, NULL);
    sigqueue(getpid(), SIGUSR1, info);
  //  sigaction(SIGUSR1, &sig, NULL);
  //  raise(SIGUSR1);
    //  __builtin_trap();
}


#include "preproc.h"
#define sleep_between false //sleep between starting workers

sem_t** semid;
int shmid;
struct timeval tv;
int* shmem;

void increase(int sem_num){
    if(sem_post(semid[sem_num]) < 0) error("increase");
}

void close_sem(){
    LOG("closing all...\n");
    char name[30];
    for_i_up_to(SEM_SIZE){
       // if(semid[i] != NULL){
            sem_close(semid[i]);
            sprintf(name, "/sem %d", i);
            if(sem_unlink(name) < 0) print("failed to unlink %s\n", name);
            semid[i] = NULL;
      //  }
    }
    shm_unlink(MEM_NAME);
}

void open_sem(){
    char name[30];
    semid = new(SEM_SIZE, sizeof(sem_t*));
    for_i_up_to(SEM_SIZE){
        sprintf(name, "/sem %d", i);
        int val = 0;
        if(i==ORDERS_LEFT) val = TOTAL_ORDERS;
        if(i==ORDERS_LEFT) val = TOTAL_ORDERS;
        if(i==FREE-1 or i==ORDERS_START-1 or i==PACKAGES_START-1) val = 1;
        semid[i] = sem_open(name, O_CREAT | O_EXCL, 0666, val);
        if(semid[i] == SEM_FAILED) error("sem_open");
    }
}

void init(){
    open_sem();

    int fd = shm_open(MEM_NAME, O_CREAT | O_EXCL | O_RDWR, 0666);
    if(fd < 0) error("shm_open");
    if(ftruncate(fd, MEM_SIZE * sizeof(int)) < 0) error("ftruncate");

    srand(time(NULL));
}



int main(int argc, char** argv) {
    print("Total orders = %d\n", TOTAL_ORDERS);
    print("Memory size = %d\n", MEM_SIZE);
    print("Number of workers I = %d\n", FRST_NUM);
    print("Number of workers II = %d\n", SCND_NUM);
    print("Number of workers III = %d\n\n", THRD_NUM);
    init();

    for_i_up_to(FRST_NUM){
        print("starting first worker...\n");
        pid_t pid = fork();
        if (pid == 0){
            increase(FRST_WORKING); // number of active workers
            char* args[]={"./first",NULL};
            execv(args[0],args); 
        }
    }

    if(sleep_between) sleep(2*FRST_NUM);

    for_i_up_to(SCND_NUM){
        print("starting second worker...\n");
        pid_t pid = fork();
        if (pid == 0){
            increase(SCND_WORKING); // number of active workers
            char* args[]={"./second",NULL};
            execv(args[0],args); 
        }
    }

    if(sleep_between) sleep(2*FRST_NUM);

    for_i_up_to(THRD_NUM){
        print("starting third worker...\n");
        pid_t pid = fork();
        if (pid == 0){
            increase(THRD_WORKING);
            char* args[]={"./third",NULL};
            execv(args[0],args); 

        }
    }

    atexit(close_sem);
    signal(SIGINT, close_sem);

    sleep(1);
    int third = 1;
    while(third != 0){
        safe_call(sem_getvalue(semid[THRD_WORKING], &third));
    } 

    LOG("Main ended\n");
}
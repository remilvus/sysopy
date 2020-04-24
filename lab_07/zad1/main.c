#include "preproc.h"
#define sleep_between false //sleep between starting workers

int semid;
int shmid;
struct sembuf decrease_buf;
struct sembuf increase_buf;
struct shmid_ds shmem_op;
struct timeval tv;
int* shmem;

int get_diff(int start, int end){
  //  print("diff %d %d |start + MEM_SIZE - end = %d\n", start, end, start + MEM_SIZE - end);
    return (end - start + MEM_SIZE) % MEM_SIZE;
}

void decrease(int sem_num){
    decrease_buf.sem_num = sem_num;
    //LOG("decrease %d\n",sem_num);
    if(semop(semid, &decrease_buf, 1) < 0) error("decrease");
}

void increase(int sem_num){
    increase_buf.sem_num = sem_num;
    //LOG("increase %d\n",sem_num);
    safe_call(semop(semid, &increase_buf, 1));
}

void block(int sem_num){
    decrease(sem_num - 1);
    //LOG("%d block %d\n", getpid(), sem_num);
}

void unblock(int sem_num){
    //LOG("%d unblock %d\n", getpid(), sem_num);
    increase(sem_num - 1);
}

void set_next_idx(int semnum, int current){
    if(current + 1 == MEM_SIZE){
        union semun op;
        op.val = 0;
        if(0 > semctl(semid, semnum, SETVAL, op)) error("semctl, set_next_idx");
    } else {
        increase(semnum);
    }
}

void close_sem(){
    if(semid >= 0){
        semctl(semid, 0, IPC_RMID);
        semid = -1;
    }
    if(shmid >= 0){
        shmdt(shmem);
        shmctl(shmid, IPC_RMID, NULL);
        shmid = -1;
    }
}

void init(){
 //   atexit(close_sem);
    key_t key = ftok(getenv("HOME"), PROJECT_ID);
    if (key == -1)   error("ftok");

    semid = semget(key, SEM_SIZE,IPC_EXCL | IPC_CREAT | 0666);
    if (semid < 0)     error("semget");

    shmid = shmget(key, MEM_SIZE*sizeof(int), IPC_EXCL | IPC_CREAT | 0666); 
    if (shmid < 0)     error("shmget");
    LOG("shmid %d\n", shmid);

    shmem = shmat(shmid, NULL, 0);
    if ((long int)shmem == -1)     error("shmat");

    union semun init;
    init.val = 0;
    if(0 > semctl(semid, FRST_WORKING, SETVAL, init)) error("semctl");
    if(0 > semctl(semid, SCND_WORKING, SETVAL, init)) error("semctl");
    init.val = TOTAL_ORDERS;
    if(0 > semctl(semid, ORDERS_LEFT, SETVAL, init)) error("semctl");

    decrease_buf.sem_op = -1;
    decrease_buf.sem_flg = 0;
    increase_buf.sem_op = 1;
    increase_buf.sem_flg = 0;

    unblock(FREE);
    unblock(ORDERS_START);
    unblock(PACKAGES_START);

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
    // int third = semctl(semid, THRD_WORKING, GETVAL);
    // int second_working = semctl(semid, SCND_WORKING, GETVAL);
    // print("III: %d II: %d\n", third, second_working);
    struct sembuf buf;
    buf.sem_op = 0;
    buf.sem_num = THRD_WORKING;
    if(semop(semid, &buf, 1) < 0) print("semop\n");

    LOG("Main ended\n");
}
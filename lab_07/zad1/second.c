#include "preproc.h"

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

void memdet(){
    if (shmem != NULL){
        shmdt(shmem);
        shmem = NULL;
    }
    exit(0);
}


void second_worker(){
    block(ORDERS_START);
    while(true){

        int free_idx = semctl(semid, FREE, GETVAL);
        int orders_idx = semctl(semid, ORDERS_START, GETVAL);
        int package_idx = semctl(semid, PACKAGES_START, GETVAL);

        if(free_idx == orders_idx){ // no unprepared packages
            unblock(ORDERS_START);
            int first_working = semctl(semid, FRST_WORKING, GETVAL);

            if(first_working == 0){ // all first workers stopped
                free_idx = semctl(semid, FREE, GETVAL);
                if(free_idx == orders_idx){ // check if there was a last update in the meantime
                    break;
                }
            }
        } else {
            LOG("II free %d | pack %d | ord %d\n", free_idx, package_idx, orders_idx);
            int n = shmem[orders_idx] * 2;
            gettimeofday(&tv,NULL); 
            print("(%d | %ld[s]%ld[ms]) Przygotowałem zamówienie o wielkości %d. Liczba zamównień \
do przygotowania: %d. Liczba zamównień do wysłania: %d.\n", getpid(), tv.tv_sec, tv.tv_usec, n, 
get_diff(orders_idx, free_idx)-1, get_diff(package_idx, orders_idx)+1);

            shmem[free_idx] = n;

            set_next_idx(ORDERS_START, orders_idx);
            unblock(ORDERS_START);
        }

        
        
        sleep(1);
        block(ORDERS_START);
    }

    decrease(SCND_WORKING);
}

int main(){
    key_t key = ftok(getenv("HOME"), PROJECT_ID);
    if (key == -1)   error("ftok");

    semid = semget(key, SEM_SIZE, IPC_PRIVATE);
    if (semid < 0)     error("semget");

    shmid = shmget(key, MEM_SIZE*sizeof(int), IPC_PRIVATE); 
    if (shmid < 0)     error("shmget");
    LOG("shmid %d\n", shmid);

    shmem = shmat(shmid, NULL, 0);
    if ((long int)shmem == -1)     error("shmat");

    decrease_buf.sem_op = -1;
    decrease_buf.sem_flg = 0;
    increase_buf.sem_op = 1;
    increase_buf.sem_flg = 0;

    atexit(memdet);
    signal(SIGINT, memdet);
    second_worker();
}
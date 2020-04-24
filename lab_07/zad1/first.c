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


void first_worker(){
    srand(getpid());
    // semum sem;
    // sem.buf = NULL;
    struct sembuf get_op;
    get_op.sem_num = ORDERS_LEFT;
    get_op.sem_op = -1;
    get_op.sem_flg = IPC_NOWAIT;
    block(FREE);
    while(semop(semid, &get_op, 1) == 0){
        //block(FREE);
        int left = semctl(semid, ORDERS_LEFT, GETVAL);
        LOG("I orders left %d\n", left);

        int free_idx = semctl(semid, FREE, GETVAL);
        int package_idx = semctl(semid, PACKAGES_START, GETVAL);
        int orders_idx = semctl(semid, ORDERS_START, GETVAL);
        LOG("I free %d | pack %d | ord %d\n", free_idx, package_idx, orders_idx);
        if(free_idx == package_idx and 
            not (free_idx == orders_idx and free_idx ==  package_idx)){ // no free space
            increase(ORDERS_LEFT);
            unblock(FREE);
        } else {
            int n = rand() % RAND_CEIL;
            shmem[free_idx] = n;

          //  set_next_idx(FREE, free_idx);
            while((free_idx + 1) % MEM_SIZE == package_idx){ // next index is already taken
                //sleep(1);
                package_idx = semctl(semid, PACKAGES_START, GETVAL);
            }
            set_next_idx(FREE, free_idx);
            gettimeofday(&tv,NULL); 
            int package_idx = semctl(semid, PACKAGES_START, GETVAL);
            int orders_idx = semctl(semid, ORDERS_START, GETVAL);
            print("(%d | %ld[s]%ld[ms]) Dodałem liczbę: %d. Liczba zamówień do przygotowania:\
%d. Liczba zamówień do wysłania: %d.\n", getpid(), tv.tv_sec, tv.tv_usec, n, 
    get_diff(orders_idx, free_idx)+1, get_diff(package_idx, orders_idx));
            unblock(FREE);
        }

        
        
      //  sleep(1);
        block(FREE);
    }
    unblock(FREE);

    decrease(FRST_WORKING);
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
    first_worker();
}
#include "preproc.h"

sem_t** semid;
struct timeval tv;
int* shmem;

int get_diff(int start, int end){
  //  print("diff %d %d |start + MEM_SIZE - end = %d\n", start, end, start + MEM_SIZE - end);
    return (end - start + MEM_SIZE) % MEM_SIZE;
}

void decrease(int sem_num){
    if(sem_wait(semid[sem_num]) < 0) error("decrease");
}

void increase(int sem_num){
    if(sem_post(semid[sem_num]) < 0) error("increase");
}

void block(int sem_num){
    decrease(sem_num - 1);
}

void unblock(int sem_num){
    increase(sem_num - 1);
}

void set_next_idx(int semnum, int current){
    if(current + 1 == MEM_SIZE){
        while(sem_trywait(semid[semnum]) != -1){}
    } else {
        increase(semnum);
    }
}

void first_worker(){
    srand(getpid());
    // semum sem;
    // sem.buf = NULL;

    block(FREE);
    while(sem_trywait(semid[ORDERS_LEFT]) == 0){
        //block(FREE);
        int left;
        safe_call(sem_getvalue(semid[ORDERS_LEFT], &left));
        LOG("I orders left %d\n", left);

        int free_idx;
        safe_call(sem_getvalue(semid[FREE], &free_idx));
        int package_idx;
        safe_call(sem_getvalue(semid[PACKAGES_START], &package_idx));
        int orders_idx;
        safe_call(sem_getvalue(semid[ORDERS_START], &orders_idx));
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

                safe_call(sem_getvalue(semid[PACKAGES_START], &package_idx));
            }
            set_next_idx(FREE, free_idx);
            gettimeofday(&tv,NULL); 
            safe_call(sem_getvalue(semid[PACKAGES_START], &package_idx));
            safe_call(sem_getvalue(semid[ORDERS_START], &orders_idx));
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

void memdet(){
    for_i_up_to(SEM_SIZE){
        if(semid[i] != NULL){
            sem_close(semid[i]);
            semid[i] = NULL;
        }
    }
    if (shmem != NULL){
        munmap(shmem, sizeof(int)*MEM_SIZE);
        shmem = NULL;
    }
    exit(0);
}


void open_sem(){
    char name[30];
    semid = new(SEM_SIZE, sizeof(sem_t*));
    for_i_up_to(SEM_SIZE){
        sprintf(name, "/sem %d", i);
        semid[i] = sem_open(name, O_CREAT | O_RDWR);
        if(semid[i] < 0) error("worker sem_open");
    }
}

int main(){
    open_sem();

    int fd = shm_open(MEM_NAME, O_RDWR, 0666);

    shmem = mmap(NULL, MEM_SIZE*sizeof(int), PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0); 
    if ((long int)shmem == -1)     error("mmap");

    atexit(memdet);
    signal(SIGINT, memdet);
    first_worker();
}
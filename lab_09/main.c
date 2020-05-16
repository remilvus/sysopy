#include "preproc.h"

typedef struct barber_s{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    bool is_sleeping;
} barber_s;

typedef struct clients_s{
    int total;
    int count;
    int num_chairs;
    int* chairs;
    pthread_mutex_t mutex;
    bool all_arrived;
} clients_s;

barber_s barber;
clients_s clients;


void* worker(void* arg){
    pthread_mutex_lock(&barber.mutex);
    while(!clients.all_arrived or clients.count > 0){
        while(clients.count == 0){
                print("Golibroda: ide spac\n");
                barber.is_sleeping = true;
                pthread_cond_wait(&barber.cond, &barber.mutex);
                barber.is_sleeping = false;
            }
        pthread_mutex_lock(&clients.mutex);
        clients.count--;
        int id;
        for_i_up_to(clients.num_chairs){
            if (clients.chairs[i] != -1){
                id = clients.chairs[i];
                clients.chairs[i] = -1;
                break;
            }
        }
        print("Golibroda: czeka %d klientow, gole klienta ID %d\n",clients.count, id);
        pthread_mutex_unlock(&clients.mutex);
        sleep(rand() % WORK_MAX_TIME + 1);

    }
    pthread_mutex_unlock(&barber.mutex);
    return NULL;

}


void* client_thread(void* arg){
    int id = *((int*)arg);
    free(arg);
    start:;
    pthread_mutex_lock(&clients.mutex);
    int block_barb = pthread_mutex_trylock(&barber.mutex);
    if(block_barb == EBUSY){
        if(clients.count == clients.num_chairs){
            print("Zajete; ID %d\n", id);
            pthread_mutex_unlock(&clients.mutex);
            sleep(rand() % MAX_WAIT + 1);
            goto start;
        }
        for_i_up_to(clients.num_chairs){
            if(clients.chairs[i]==-1) {clients.chairs[i]=id; break;}
        }
        clients.count++;
        print("poczekalnia, wolne miejsca: %d; ID, %d\n", clients.num_chairs - clients.count, id);
    } else {
        if(block_barb==0){
            if(clients.chairs[0] != -1 or clients.count != 0) error("there is other clients and barber sleeps\n");
            clients.count++;
            clients.chairs[0] = id;
            print("Budze golibrode; ID %d\n", id);
            pthread_cond_broadcast(&barber.cond);
            pthread_mutex_unlock(&barber.mutex);
        } else {
            error("pthread_mutex_trylock returned unexpected value\n");
        }
    }
    pthread_mutex_unlock(&clients.mutex);
    return NULL;
}



int main(int argc, char** argv) {
    srand(getpid());
    pthread_t t1,t2;
    if(argc < 3) error("not enough arguments\n");
    clients.num_chairs = atoi(argv[1]); 
    clients.total = atoi(argv[2]);
    if (clients.num_chairs <= 0 or clients.total <= 0) error("Bad argument\n");
    clients.chairs = new(clients.num_chairs, sizeof(int));
    for_i_up_to(clients.num_chairs) clients.chairs[i] = -1;
    clients.all_arrived = false;

    pthread_cond_init(&barber.cond, NULL);
    pthread_mutex_init(&barber.mutex, NULL);
    pthread_mutex_init(&clients.mutex, NULL);

    LOG("create worker\n");
    pthread_create(&t1, NULL, &worker, NULL);

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    for_i_up_to(clients.total){
        sleep((rand() % MAX_WAIT + 1)/2);
        LOG("create client %d\n", i);
        int* id = new(1, sizeof(int));
        *id = i;
        pthread_create(&t2, &attr, &client_thread, (void*)id);
        
    }
    pthread_attr_destroy(&attr);
    clients.all_arrived = true;
    safe_call(pthread_join(t1, NULL));
    
}
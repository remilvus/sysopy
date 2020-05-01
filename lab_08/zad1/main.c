#include "preproc.h"

int h;
int w;
image_array image;
int hist[256];
int semid;

void block(int sem_num){
    struct sembuf decrease_buf;
    decrease_buf.sem_num = sem_num;
    decrease_buf.sem_op = -1;
    decrease_buf.sem_flg = 0;
    //LOG("decrease %d\n",sem_num);
    if(semop(semid, &decrease_buf, 1) < 0) error("decrease");
}

void unblock(int sem_num){
    struct sembuf increase_buf;
    increase_buf.sem_num = sem_num;
    increase_buf.sem_op = 1;
     increase_buf.sem_flg = 0;

    safe_call(semop(semid, &increase_buf, 1));
}

void* sign_worker(void* arg){
    worker_info* info = (worker_info*) arg;
    unsigned long* elapsed = new(1,unsigned long);
    struct timeval start, end;
    gettimeofday(&start,NULL);

    int part = 1 + 255 / info->workers;
    int min = part * info->id;
    int max = min + part;

    if(min < 256){
        for_i_up_to(w){
            for_j_up_to(h){
                int pix = image[i][j];
                if(min <= pix and pix < max){
                    hist[pix] += 1;
                }
            }
        }
    }

    gettimeofday(&end, NULL);
    *elapsed = 1000000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
    pthread_exit(elapsed);
}

void* block_worker(void* arg){
    worker_info* info = (worker_info*) arg;
    unsigned long* elapsed = new(1,unsigned long);
    struct timeval start, end;
    gettimeofday(&start,NULL);

    int part = (w + info->workers - 1) / info->workers;
    int left = part * info->id;
    int right = left + part;
    right = min(right, w-1);

    if(left < w){
        for(int i=left; i<right; i++){
            for_j_up_to(h){
                int pix = image[j][i];
                block(pix);
                hist[pix] += 1;
                unblock(pix);
            }
        }
    }

    gettimeofday(&end, NULL);
    *elapsed = 1000000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
    pthread_exit(elapsed);
}

void* interleaved_worker(void* arg){
    worker_info* info = (worker_info*) arg;
    unsigned long* elapsed = new(1,unsigned long);
    struct timeval start, end;
    gettimeofday(&start,NULL);

    int jump = info->workers;

    if(info->id < w){
        for(int i=info->id; i < w; i+=jump){
            for_j_up_to(h){
                int pix = image[j][i];
                block(pix);
                hist[pix] += 1;
                unblock(pix);
            }
        }
    }


    gettimeofday(&end, NULL);
    *elapsed = 1000000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
    pthread_exit(elapsed);
}

void close_sem(){
    if(semid >= 0){
        semctl(semid, 0, IPC_RMID);
        semid = -1;
    }
}

void load_image(string input){
    FILE * fp;
    fp = fopen (input, "r");
    if (fp <= 0) error("failed to open input file");
    
    char* line = NULL;
    size_t len = 0;

    safe_call(getline(&line, &len, fp)); // ignore
    safe_call(getline(&line, &len, fp)); // size
    w = atoi(strtok(line, " "));
    h = atoi(strtok(NULL, " "));
    if(w <= 0 or h <= 0) error("wrong dimesions of input image\n");
   // print("loading '%s'...   %dx%d\n", input, w, h);
    safe_call(getline(&line, &len, fp)); // ignore
    image = new(h, sizeof(int*));

    for_i_up_to(h){
        image[i] = new(w, sizeof(int*));
        for_j_up_to(w){
            image[i][j] = getc(fp);
            if (image[i][j] == -1) error("failed to read pixel\n");
        }
    }

    fclose (fp);
    if(line) free(line);
}

int main(int argc, char** argv) {
    struct timeval start, end;
    gettimeofday(&start,NULL);
    // loading arguments
    if (argc < 5) {
        print("not enough arguments");
        exit(-1);
    }
    int thread_num = atoi(argv[1]);
    if(thread_num <= 0) {
        print("incorrect arg");
        exit(-1);
    }
    int work_type;
    if(strcmp(argv[2], "sign")==0){
        work_type = SIGN;
    } else if(strcmp(argv[2], "block")==0){
        work_type = BLOCK;
    } else if(strcmp(argv[2], "interleaved")==0) {
        work_type = INTERLEAVED;
    } else {
        print("incorrect arg");
        exit(-1);
    }
    string input = argv[3];
    if(strlen(input) <= 0) {
        print("incorrect arg");
        exit(-1);
    }
    string output = argv[4];
    if(strlen(output) <= 0) {
        print("incorrect arg");
        exit(-1);
    }
    for_i_up_to(256){
        hist[i] = 0;
    }

    load_image(input);
    pthread_t* threads = new(thread_num, sizeof(pthread_t));
    unsigned long* results = new(thread_num, sizeof(unsigned long));

    key_t key = ftok ("/tmp", 12345);
    if (key == -1)   error("ftok");
    atexit(close_sem);
    signal(SIGINT, close_sem);
    semid = semget(key, 256, IPC_EXCL | IPC_CREAT | 0666);
    if (semid < 0)    error("semget");

    union semun init;
    init.val = 1;
    for_i_up_to(256) if(0 > semctl(semid, i, SETVAL, init)) error("semctl");

    worker_info* info = new(thread_num, sizeof(worker_info));

    //todo set info
    // starting threads
    for_i_up_to(thread_num){
        info[i].id = i;
        info[i].workers = thread_num;
        switch (work_type)
        {
        case SIGN:
            pthread_create(&threads[i], NULL, &sign_worker, &info[i]);
            break;
        case INTERLEAVED:
            pthread_create(&threads[i], NULL, interleaved_worker, &info[i]);
            break;
        case BLOCK:
            pthread_create(&threads[i], NULL, block_worker, &info[i]);
            break;
        }
    }
    // todo
    for_i_up_to(thread_num){
        void* time;
        safe_call(pthread_join(threads[i], &time));
        results[i] = *((unsigned long*)time);
    }

    unsigned long elapsed;
    gettimeofday(&end, NULL);
    elapsed = 1000000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
    print("\tTryb: %s | Wątki: %d\n", argv[2], thread_num);
    print("\tCzas całkowity: %lu[us]\n", elapsed);
    // save and print
    for_i_up_to(thread_num){
        print("\t\tWątek %lu | czas: %lu[us]\n", threads[i], results[i]);
    }

    FILE * fp;
    fp = fopen (output,"w+");
    if (fp <= 0) error("failed to open result file");
    
    for_i_up_to(256){
        fprintf (fp, "#%d | %d\n", i, hist[i]);
    }
    print("\n");
    fclose (fp);
}
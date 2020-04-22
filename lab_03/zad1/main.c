
#include "preproc.h"


time_t now;
int g_atime;
int g_mtime;
int g_maxdepth;
int g_use_atime;
int g_use_mtime;
int g_use_maxdepth;
char* search_for;
char* initial_dir;
char* directory;


void process_args(int argc, char** argv, int* maxdepth, int* mtime, int* atime, int* use_maxdepth, int* use_mtime, int* use_atime){
    for(int i=1; i<argc-1; i++){
        if(strcmp(argv[i], "-maxdepth") ==0){
            *use_maxdepth = 1;
            *maxdepth = atoi(argv[i+1]); 
        }
        else if(strcmp(argv[i], "-mtime") ==0){
            *use_mtime = 1;
            *mtime = atoi(argv[i+1]); 
        }
        else if(strcmp(argv[i], "-atime") ==0){
            *use_atime = 1;
            *atime = atoi(argv[i+1]); 
        }
    }
}

char* get_type(const struct stat* s){
    char* name = new(15, char);
    if((s->st_mode & __S_IFMT) == __S_IFSOCK){
        strcpy(name, "sock");
    } else if(S_ISLNK(s->st_mode)){
        strcpy(name, "slink");
    } else if(S_ISREG(s->st_mode)){
        strcpy(name, "file");
    } else if(S_ISBLK(s->st_mode)){
        strcpy(name, "block dev");
    } else if(S_ISDIR(s->st_mode)){
        strcpy(name, "dir");
    } else if(S_ISCHR(s->st_mode)){
        strcpy(name, "char dev");
    }else if(S_ISFIFO(s->st_mode)){
        strcpy(name, "FIFO");
    }else {
        strcpy(name, "other");
    }

    return name;
}

int is_above(char* dir){
    return strcmp(dir, "..") == 0;
}

int is_dot(char* dir){
    return dir[(strlen(dir)-1)] == '.';
}

int is_first_dot(char* dir){
    return dir[(strlen(dir)-1)] == '.' and strlen(dir) == 1;
}


void clearLast(char* dir, int size){
    short flag = 1;
    size -= 1;
    while(flag and size >= 0){
        flag = not (dir[size] == '/');
        dir[size] = 0;
        size -=1;
    }
    if (size == -1){
        printf("No such directory");
        exit(-1);
    }
}

void search(char* dir_name, int level, int maxdepth, int mtime, int atime, int use_maxdepth, int use_mtime, int use_atime){
    if(not is_first_dot(dir_name) and is_dot(dir_name)) return;

    printf("%s\n", dir_name);
    pid_t cp = vfork();
    if ((int)cp == 0){
        printf("child process pid: %d\n", (int)getpid());
        char* command = new(256, char);
        sprintf(command, "ls %s -l | cat", dir_name);
        system(command);
        printf("\n");
        free(command);
        exit(0);
    }

    DIR* dir = opendir(dir_name);
    struct dirent* content = readdir(dir); 
    while(content != NULL){
        if(not is_above(content->d_name) and content->d_type == 4){
            char* full_name = new(strlen(dir_name) + strlen(content->d_name) + 5, char);
            
            strcpy(full_name, dir_name);
            strcat(full_name, "/");
            strcat(full_name, content->d_name);
            
            if (not is_dot(full_name) and (level<maxdepth or not use_maxdepth)){ // directory
                char* next_dir = new(strlen(dir_name) + strlen(content->d_name) + 5, char);
                strcpy(next_dir, dir_name);
                strcat(next_dir, "/");
                strcat(next_dir, content->d_name);
                search(next_dir, level+1, maxdepth, mtime, atime, use_maxdepth, use_mtime, use_atime);
                free(next_dir);
            }
            free(full_name);

        }
        content = readdir(dir); 
    }
}

int main(int argc, char **argv){
    int maxdepth = -1;
    int mtime = -1;
    int atime = -1;
    int use_mtime = 0;
    int use_atime = 0;
    int use_maxdepth = 0;
    process_args(argc, argv, &maxdepth, &mtime, &atime, &use_maxdepth, &use_mtime, &use_atime);
    if(use_mtime or use_atime){
        time(&now);
    }
    char* dir = ".";
    
    if(maxdepth==0)exit(0);
   
    search(dir, 1, maxdepth, mtime, atime, use_maxdepth, use_mtime, use_atime);
}
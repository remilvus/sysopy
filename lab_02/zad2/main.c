
#include "preproc.h"


#define USE_NFTW false

time_t now;
int g_atime;
int g_mtime;
int g_maxdepth;
int g_use_atime;
int g_use_mtime;
int g_use_maxdepth;
char* search_for;
char* initial_dir;


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

char* get_name(const char* path){
    return strrchr(path, '/') + sizeof(char);
}

int get_level(const char* file){
    int level = 0;
    int i = strlen(initial_dir);
    for(;i < strlen(file); i++){
        if(file[i] == '/') level++;
    }
    return level;
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

int is_special_dir(char* dir){
    return strcmp(dir, ".") == 0 or strcmp(dir, "..") == 0;
}

int is_time_correct(const time_t* mtime_st,const time_t* atime_st, int* use_mtime, int* use_atime, int* atime, int* mtime){
    if(*use_atime){
        double dif = difftime(*atime_st, now);
        int day_dif = (int)(dif / (24*60*60));
        printf("%d\n", day_dif);
        if( (*atime)*dif < 0 or abs(day_dif) > abs(*atime) ) return false;
    }
    if(*use_mtime){
        double dif = difftime(*mtime_st, now);
        int day_dif = (int)(dif / (60*60*24));
        printf("%d\n", day_dif);
        if( (*mtime)*dif < 0 or abs(day_dif) > abs(*mtime) ) return false;
    }
    return true;
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

void search(char* dir_name, char* file_name, int level, int maxdepth, int mtime, int atime, int use_maxdepth, int use_mtime, int use_atime){
    DIR* dir = opendir(dir_name);
    struct dirent* content = readdir(dir); 
    while(content != NULL){
        if(not is_special_dir(content->d_name)){
            if (content->d_type == 4 and (level<maxdepth or not use_maxdepth)){ // directory
                char* next_dir = new(strlen(dir_name) + strlen(content->d_name) + 5, char);
                strcpy(next_dir, dir_name);
                strcat(next_dir, "/");
                strcat(next_dir, content->d_name);
                search(next_dir, file_name, level+1, maxdepth, mtime, atime, use_maxdepth, use_mtime, use_atime);
                free(next_dir);
            }
            if (strcmp(file_name, content->d_name) == 0 or strlen(file_name)==0){
                // create char array with full name
                printf("%s/ %s\n", dir_name, content->d_name);
                char* full_name = new(strlen(dir_name) + strlen(content->d_name) + 5, char);
                strcpy(full_name, dir_name);
                strcat(full_name, "/");
                strcat(full_name, content->d_name);
                
                //get stats
                struct stat* stats = new(1, struct stat);
                stat(full_name, stats);

                int size = 16;
                char* mdate = new(size, char);
                char* adate = new(size, char);
                struct tm* tm_mdate = localtime(&stats->st_mtime);
                strftime(mdate, size,"%Y-%m-%d" ,tm_mdate);
                struct tm* am_mdate = localtime(&stats->st_atime);
                strftime(adate, size,"%Y-%m-%d", am_mdate);
                char* type = get_type(stats);
                if(is_time_correct(&(stats->st_mtime), &(stats->st_atime), &use_mtime, &use_atime, &atime, &mtime)){
                    //print everything
                    printf("%s/%s\n\tlinks: %ld\n\ttype: %s\n", dir_name, content->d_name, stats->st_nlink, type);
                    printf("\tsize: %ld\n\tmtime %s\n\tatime: %s\n", stats->st_size, mdate, adate);
                }
                free(full_name);
                free(adate);
                free(mdate);
                free(type);
            }
        }
        content = readdir(dir); 
    }
}

int nftw_fun(const char *file_name, const struct stat *stats, int typeflag, struct FTW *ftwbuf){
    if(get_level(file_name) > g_maxdepth and g_use_maxdepth) return 0;
    char* file = get_name(file_name); // only name of file

    if (strcmp(search_for, file) == 0 or strlen(search_for)==0){
        int size = 16;
        char* mdate = new(size, char);
        char* adate = new(size, char);
        struct tm* tm_mdate = localtime(&stats->st_mtime);
        strftime(mdate, size,"%Y-%m-%d" ,tm_mdate);
        struct tm* am_mdate = localtime(&stats->st_atime);
        strftime(adate, size,"%Y-%m-%d", am_mdate);
        char* type = get_type(stats);
        if(is_time_correct(&(stats->st_mtime), &(stats->st_atime), &g_use_mtime, &g_use_atime, &g_atime, &g_mtime)){
        //print everything
        printf("%s\n\tlinks: %ld\n\ttype: %s\n", file_name, stats->st_nlink, type);
        printf("\tsize: %ld\n\tmtime %s\n\tatime: %s\n", stats->st_size, mdate, adate);
        }

        free(adate);
        free(mdate);
        free(type);
    }

    return 0;
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
    char* dir = argv[1];
    char *file = argv[2];
    if(strcmp(file, "-noname")==0) file[0]=0;

    short special_dir = false;
    if(strcmp(dir, ".") == 0){
        special_dir = true;
        dir = new(256, char);
        dir = getcwd(dir, 256);
    }
    if(strcmp(dir, "..") == 0){
        special_dir = true;
        dir = new(256, char);
        dir = getcwd(dir, 256);
        clearLast(dir, 256);
    }


    if(dir == NULL){
        printf("Can't get initial directory");
        exit(-1);
    }
 //   printf("initial dir:\n%s\n\n", dir);
    if(maxdepth==0)exit(0);

    if(USE_NFTW){
        g_atime = atime;
        g_mtime = mtime;
        g_maxdepth = maxdepth;
        g_use_atime = use_atime;
        g_use_mtime = use_mtime;
        g_use_maxdepth = use_maxdepth;
        search_for = file;
        initial_dir = dir;
        nftw(dir, nftw_fun, getdtablesize() - 5, 1);
    } else {
        search(dir, file, 1, maxdepth, mtime, atime, use_maxdepth, use_mtime, use_atime);
    }

    if(special_dir) free(dir);
}
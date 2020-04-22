
#include "preproc.h"

#define OUT_ONE true

typedef struct matrix{
    int w;
    int h;
    long long** data;
} matrix;

typedef struct matricies{
    matrix* left;
    matrix* right;
    matrix* out;
} matricies;

double timeDiff(clock_t start, clock_t end){
    return ((double)(end - start) / sysconf(_SC_CLK_TCK));
}

string setLast(string names){
    string start = strchr(names, ' ');
    *start = '_';
    start = strchr(start, ' ');
    start += c_size;

    return start;
}

void delet(matricies* mat){

    for_i_up_to(mat->out->h){
        free(mat->out->data[i]);
    }
    for_i_up_to(mat->right->h){
        free(mat->right->data[i]);
    }
    for_i_up_to(mat->left->h){
        free(mat->left->data[i]);
    }

    free(mat->out->data);
    free(mat->right->data);
    free(mat->left->data);
    free(mat);
}

matrix* read_size(string filename){
    FILE* file = fopen(filename, "r");

    matrix* mat = new(1, matrix);
    if(mat==NULL){
        printf("CANT ALLOCATE\n");
        exit(-1);
    }

    char* line = new(30, char);
    if(line==NULL){
        printf("CANT ALLOCATE\n");
        exit(-1);
    }
    fgets(line, 30, file);
    mat->h = atoi(line);
    memset(line, 0, 30);
    fgets(line, 30, file);
    mat->w = atoi(line);
  //  free(line);
    fclose(file);

    return mat;
}

void fill(string filename, matrix* mat){
    if(access(filename, F_OK ) == -1){
        FILE* file = fopen(filename, "w+");
        int res = flock(fileno(file), LOCK_EX | LOCK_NB);
        if(res==-1)return;

        int size = mat->w*COL_WIDTH;
        char* line = new(size, char);
        if(line==NULL){
            printf("CANT ALLOCATE\n");
            exit(-1);
        }
        memset(line, 'x', size);
        line[size-1]='\n';
        for_i_up_to(mat->h){
            fputs(line, file);
        }

        flock(fileno(file), LOCK_UN);
        fclose(file);

       // free(line);
    }
}

void multiply(matricies* mat, int start, int end){
    for_i_up_to(mat->left->h){
        for(int k=start; k<=end; k++){
            long long sum = 0;
            for_j_up_to(mat->left->w){
                sum += mat->left->data[i][j] * mat->right->data[j][k]; 
            }
            mat->out->data[i][k] = sum;
        }
    }
}

void load_matrix(string filename, int start, int end, matrix* mat){
    FILE* file = fopen(filename, "r");
    flock(fileno(file), LOCK_SH);

    // matrix allocation
    mat->data = new(mat->h, long long*);
    if(mat->data==NULL){
        printf("CANT ALLOCATE MATRIX\n");
        exit(-1);
    }
    for_i_up_to(mat->h){
        mat->data[i] = new(end - start,long long);
        if(mat->data[i]==NULL){
            printf("CANT ALLOCATE MATRIX\n");
            exit(-1);
        }
    }

    int size = COL_WIDTH*mat->w+1;

    char* line = new(size, char);

    if(line==NULL){
        printf("CANT ALLOCATE\n");
        exit(-1);
    }
    // skip sizes
    fgets(line, size, file);
    fgets(line, size, file);

    fgets(line, size, file);
    for_i_up_to(mat->h){
        char* col_start = line + start*COL_WIDTH*c_size;
        for(int j=start; j<=end; j++){
            char* col_end = col_start + (COL_WIDTH-1)*c_size;
            *col_end=0;
            long long num = atoi(col_start);
            mat->data[i][j] = num;
            col_start = col_end + c_size;
        }
        fgets(line, size, file);
    }

    flock(fileno(file), LOCK_UN);
    fclose(file);
  //  free(line);
}

void print_matrix(matrix* mat, int start, int end){
    for_i_up_to(mat->h){

        for(int j=start; j<=end; j++){
            printf("%lld ", mat->data[i][j]);
        }
        printf("\n");
    }
     printf("\n");
}

void save_matrix(string filename, int id, matrix* mat,int start, int end){

    if (not OUT_ONE){
        char* newname = new(150, char);
        if(newname==NULL){
            printf("CANT ALLOCATE\n");
            exit(-1);
        }
        filename[strlen(filename)-4] = 0;
        sprintf(newname, "%s-%d.txt", filename, id);
  //      printf("save to: >%s<\n", newname);
        FILE* out = fopen(newname, "w+");

        int size = (end-start+1)*COL_WIDTH + 1;
        char* line = new(size, char);
        if(line==NULL){
            printf("CANT ALLOCATE\n");
            exit(-1);
        }
        char* col_start;

        for_i_up_to(mat->h){
            col_start = line;
            memset(line, ' ', size);
            line[size-1]=0;
            for(int j=start; j<=end; j++){
                sprintf(col_start, "%lld", mat->data[i][j]);
                *(col_start + c_size*strlen(col_start)) = ' ';
                col_start += COL_WIDTH*c_size;
            }
            line[size-2]='\n';
            fputs(line, out);
        }

        fclose(out);
      //  free(newname);
     //   free(line);
    }else{
        
        FILE* file = fopen(filename, "r+");
        if(file==NULL){
            printf("ERROR: file %s couldn't be opened\n", filename);
            exit(-1);
        }

        int res = flock(fileno(file), LOCK_EX);
        if(res==-1){
            printf("CAN'T ACQUIRE LOCK (PID %d)", getpid());
            exit(-1);
        }

        string col = new(COL_WIDTH, char);
        if(col==NULL){
            printf("CANT ALLOCATE\n");
            exit(-1);
        }

        for_i_up_to(mat->h){
            for(int j=start; j<=end; j++){
                memset(col, ' ', COL_WIDTH);
                sprintf(col, "%lld ", mat->data[i][j]);
                *(col + c_size*strlen(col)) = ' ';
                if(j==mat->w-1) col[COL_WIDTH-1] = '\n';
                fseek(file, (i*mat->w + j)*COL_WIDTH, SEEK_SET);
              //  printf("write: >%s<\n", col);
                fwrite(col, c_size, COL_WIDTH, file);
            }

        }

   //     free(col);
        flock(fileno(file), LOCK_UN);
        fclose(file);
    }
}

void split(string names, string left, string right, string out){
    char* start = names;

    char* end = strchr(start, ' ');
    *end = 0;
    strcpy(left, start);
    start = end + c_size;

    end = strchr(start, ' ');
    *end = 0;
    strcpy(right, start);
    start = end + c_size;

    end = strchr(start, '\n');
    *end = 0;
    strcpy(out, start);
    start = end + c_size;
}

void process(char* filename, int id, int workers, int max_time){
    time_t start_time = time(NULL);
    time_t now;

//    printf("PROCES %d STARTS\n", id);
    FILE* input_file = fopen(filename, "r");
    if(input_file==NULL){
        printf("CANT OPEN INPUT FILE");
        exit(-1);
    }
    char* names = new(300, char);
    char* left = new(100, char);
    char* right = new(100, char);
    char* out = new(100, char);

     if(names==NULL){
        printf("CANT ALLOCATE\n");
        exit(-1);
    }
    char count=0;

    while(true){
        memset(names, 0, 300); memset(left, 0, 100); memset(right, 0, 100); memset(out, 0, 100);
        names = fgets(names, 300, input_file);
        if(names == NULL) break;
        split(names, left, right, out);
//        printf("files to proc: %s %s %s\n", left, right, out);


        matricies* mat = new(1, matricies);
        if(mat==NULL){
            printf("CANT ALLOCATE\n");
            exit(-1);
        }

        // getting matricies info
        mat->left = read_size(left);
        mat->right = read_size(right);

        mat->out = new(1, matrix);
        if(mat->out==NULL){
            printf("CANT ALLOCATE\n");
            exit(-1);
        }
        mat->out->w = mat->right->w;
        mat->out->h = mat->left->h;
        mat->out->data = new(mat->out->h, int*);
        if(mat->out->data==NULL){
            printf("CANT ALLOCATE\n");
            exit(-1);
        }
        for_i_up_to(mat->out->h){
            mat->out->data[i] = new(mat->out->w,int);
            if(mat->out->data[i]==NULL){
            printf("CANT ALLOCATE\n");
            exit(-1);
            }
        }

        // caluclate which part of right matrix to use
        int part_size = ceil((double)mat->right->w / (double)workers);
        part_size = max(1, part_size);
        int start = id * part_size;
        int end = min((id+1)*part_size - 1, mat->right->w - 1); // end is inculsive

        if(start >= mat->right->w) continue; // not enough work for this worker


        if(OUT_ONE and access(out, F_OK ) == -1){ // output must be created
            fill(out, mat->out);
        }
        
       // printf("loading matricies\n");

        load_matrix(left, 0, mat->left->w-1, mat->left);
        load_matrix(right, start, end, mat->right);

    //    if(count==1)exit(1);
    //   printf("printing matricies\n");
    //   print_matrix(mat->left, 0, mat->left->w-1);
    //   print_matrix(mat->right, start, end);

        multiply(mat, start, end);
        // print_matrix(mat->out, 0, mat->out->w-1);

        save_matrix(out, id, mat->out, start, end);

        count+=1;
        now = time(NULL);
      //  printf("time: %ld\n", (now - start_time));
        if ((now - start_time) > max_time){
            break;
        }

     //   delet(mat);
    }

   // free(names);
    fclose(input_file);

    exit(count);
}

int main(int argc, char **argv){
    char* file = argv[1];
    int num_processes = atoi(argv[2]);
    int max_sec = atoi(argv[3]);
 //   printf("col_width = %d\n", COL_WIDTH);

    for_i_up_to(num_processes){
       pid_t child = fork();
       if(child==0){
           process(file, i, num_processes, max_sec);
       }
    }

    pid_t child;
    int status;
    for_i_up_to(num_processes){
        child = wait(&status);
        printf("Process %d worked %d times\n", (int)child, WEXITSTATUS(status));
    }



    // combine partial computations

    if(not OUT_ONE){
      //  printf("CONCAT\n");
        FILE* infile = fopen(file, "r");
        string command;
        string names;
        string in;
        string out;
        string filename;
        names = new(300, char);
        names = fgets(names, 300, infile);
        while(names != NULL){
            command = new(300, char);
            in = new(100, char);
            out = new(100, char);
            filename = new(200, char);
            
       //     printf("combining out from: %s\n", names);
            split(names, in, in, out);

            int was_processed = false;
            memset(command, 0, 300);

            out[strlen(out)-4] = 0;
            for_i_up_to(num_processes){
                sprintf(filename, "%s-%d.txt", out, i);
               // printf("part: %s\n", filename);
                if( access(filename, F_OK ) != -1 ) { // file exists
                   // printf("exists\n");
                    was_processed = true;
                    strcat(command, filename);
                    strcat(command, " ");
                } 
            }
            if(was_processed){

                strcat(out, ".txt");
              //  printf("out name: %s\n", out);
                char *args[]={"./merger", command, out,NULL};

                int pid = vfork();
                if(pid==0) execv("./merger", args);
            }
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
            names = fgets(names, 300, infile);
            #pragma GCC diagnostic pop
            free(out);
            free(in);
            free(command);
            free(filename);
        }


    free(names);
    }

}
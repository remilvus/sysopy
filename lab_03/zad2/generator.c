#include "preproc.h"

void make_matrix(char* filename, int a, int b){
    FILE* file = fopen(filename, "w+");
    char* line = new(b*COL_WIDTH, char);
    char* number = new(COL_WIDTH, char);
    // save size
    sprintf(number, "%d\n%d\n", a, b);
    strcat(line, number);
    fwrite(line, sizeof(char), strlen(line), file);

    for_i_up_to(a){
        memset(line, ' ', b*COL_WIDTH);
        char* col_start = line;

        for_j_up_to(b){
            sprintf(number, "%d", rand() % (MAX_VALUE + 1));
            strcpy(col_start, number);
            *(col_start + strlen(number)*sizeof(char)) = ' ';
            col_start += sizeof(char)*COL_WIDTH;
        }

        line[b*COL_WIDTH - 1] = '\n';
        fwrite(line, sizeof(char), b*COL_WIDTH, file);
    }

    fclose(file);
}

typedef struct matrix{
    int w;
    int h;
    int** data;
} matrix;


void load_matrix(FILE* file, matrix* mat){
    mat->data = new(mat->h, int*);
    for_i_up_to(mat->h){
        mat->data[i] = new(mat->w,int);
    }

    int size = COL_WIDTH*mat->w;
    
    char* line = new(size, char);
    fgets(line, size + 1, file);
    for_i_up_to(mat->h){
        char* col_start = line;
        for(int j=0; j<mat->w; j++){
            char* col_end = col_start + (COL_WIDTH-1)*c_size;
            *col_end=0;
            int num = atoi(col_start);
            mat->data[i][j] = num;
            col_start = col_end + c_size;
        }
        fgets(line, size + 1, file);
    }
    fclose(file);
}

void make_test(){
    int a=2;
    int b=3;
    int c=2;
    FILE* file = fopen("test_inl.txt", "w+");
    char* line = new(b*COL_WIDTH, char);
    char* number = new(COL_WIDTH, char);
    // save size
    sprintf(number, "%d\n%d\n", a, b);
    strcat(line, number);
    fwrite(line, sizeof(char), strlen(line), file);

    for_i_up_to(a){
        memset(line, ' ', b*COL_WIDTH);
        char* col_start = line;
        for_j_up_to(b){
            sprintf(number, "%d", (i+1)*(j+1));
            strcpy(col_start, number);
            *(col_start + strlen(number)*sizeof(char)) = ' ';
            col_start += sizeof(char)*COL_WIDTH;
        }
        line[b*COL_WIDTH - 1] = '\n';
        fwrite(line, sizeof(char), b*COL_WIDTH, file);
    }
    fclose(file);
    free(line);free(number);
    file = fopen("test_inr.txt", "w+");
    line = new(c*COL_WIDTH, char);
    number = new(COL_WIDTH, char);
    // save size
    sprintf(number, "%d\n%d\n", b, c);
    strcat(line, number);
    fwrite(line, sizeof(char), strlen(line), file);

    for_i_up_to(b){
        memset(line, ' ', c*COL_WIDTH);
        char* col_start = line;
        for_j_up_to(c){
            sprintf(number, "%d", (i+1)*(j+1));
            strcpy(col_start, number);
            *(col_start + strlen(number)*sizeof(char)) = ' ';
            col_start += sizeof(char)*COL_WIDTH;
        }

        line[c*COL_WIDTH - 1] = '\n';
        fwrite(line, sizeof(char), c*COL_WIDTH, file);
    }
    fclose(file);

}

int assert_correct(){
    FILE* file = fopen("out.txt", "r");
    matrix m;
    m.h=2;m.w=2;
    load_matrix(file, &m);
    int gud = true;
  //  printf("%d %d %d %d\n", m.data[0][0], m.data[0][1], m.data[1][0], m.data[1][1]);
    if(m.data[0][0]!=14 or m.data[0][1]!=28 or m.data[1][0]!=28 or m.data[1][1]!=56){
        gud=false;
    }
    fclose(file);
    return gud;
}

int main(int argc, char **argv){
    srand(time(0)); 
    if(strcmp(argv[1],"mtest")==0){
        make_test();
        return 0;
    }
    if(strcmp(argv[1],"assert")==0){
        int res = assert_correct();
        if (res) printf("TEST PASSED\n");
        else printf("TEST FAILED\n");
        return 0;
    }
    for(int i=1; i<argc; i+=2){
        int a = rand() % (1 + MAX_SIZE - MIN_SIZE) + MIN_SIZE;
        int b = rand() % (1 + MAX_SIZE - MIN_SIZE) + MIN_SIZE;
        int c = rand() % (1 + MAX_SIZE - MIN_SIZE) + MIN_SIZE;
        make_matrix(argv[i], a, b);
        make_matrix(argv[i+1], b, c);
    }
}
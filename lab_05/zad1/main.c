#include "preproc.h"
#define USE_POPEN 0

int count_commands(string commands, int n){
    int count=1;
    for_i_up_to(n){
        if(commands[i]=='|') count++;
    }
    return count;
}

string* get_commands(string line, string* commands){
    if (commands==NULL) commands = new(MAX_COMMANDS, string*);
    for_i_up_to(MAX_COMMANDS){
        commands[i] = strtok(line, "|\n");
        line = NULL;
        if(commands[i] == NULL) return commands;
    }
    return commands;
}

string* split_command(string line, string* command){
    if (command==NULL) command = new(MAX_ARGS, string*);
    for_i_up_to(MAX_ARGS){
        command[i] = strtok(line, " ");
        line = NULL;
        if(command[i] == NULL) return command;
    }
    return command;
}


int main(int argc, string* argv){
    FILE* fd = fopen(argv[1], "r");
    if (!fd) {
        printf("Failed to open file\n");
        return -1;
    }

    pid_t wpid;
    int status = 0;
    string line = NULL;
    size_t len = 0;
    int res;
    res = getline(&line, &len, fd);
    string* commands_array=NULL;
    string* command_array=NULL;
    string line=NULL;
    
    FILE* old_io_file;
    int old_io[2];

    while(res != -1){

        int commands = count_commands(line, (int)len);
        if(commands > MAX_COMMANDS) {printf("too many commands"); return-1;}

        commands_array = get_commands(line, commands_array);

        for_i_up_to(commands){
            if(i==0)printf("\n");
            if(i==commands-1) printf("%s \nOUTPUT:\n", commands_array[i]);
            else printf("%s |", commands_array[i]);

            if (not USE_POPEN){
                command_array = split_command(commands_array[i], command_array);

                int io[2];
                old_io[0] = io[0]; old_io[1]=io[1];
                io[0]=io[1]=-1;
                if(pipe(io)==-1){
                    printf("pipe error\n");
                    return -1;
                }

                int pid = fork();
                if(pid==0){
                    if(i!=0) {
                        if(dup2(old_io[0], STDIN_FILENO) < 0){
                            printf("dup error\n");
                            return -1;
                        }
                    }
                    if(i!=commands-1) {
                        if(dup2(io[1], STDOUT_FILENO) < 0){
                            printf("dup error\n");
                            return  -1;
                        }
                    }
                    execvp(command_array[0], command_array);
                    exit(0);
                } 
            // sleep(1);
                if(i!=0){
                    close(old_io[0]);
                }
                close(io[1]);
            } 
            if (USE_POPEN){
                FILE* io = popen(commands_array[i], "r+");
                if (io == NULL) {
                    printf("Failed to open file\n");
                    return -1;
                }
                if (line==NULL){
                    line = new(c_size, 200);
                }



                old_io_file = io;
            }
        }

        while ((wpid = wait(&status)) > 0);

        res = getline(&line, &len, fd);
    }
}
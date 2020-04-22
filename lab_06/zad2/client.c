#include "preproc.h"

mqd_t server_que_id;
mqd_t client_que_id = -1;
int client_id;
mqd_t connected_with = -1;
int connected_with_id = -1;
string last_connection = NULL;
char name[NAME_SIZE];
unsigned int msg_prio;

void handle_message(string);

void name_me(){
    memset(name, 0, NAME_SIZE);
    sprintf(name, "/%d", getpid());
    srand(time(NULL));
    for(int i=strlen(name); i < NAME_SIZE - 1; i++){
        name[i] = 'A' + rand() % 26;
    }
}

void connect(int id){
    char msg[MSG_SIZE];
    msg[0] = client_id;
    msg[1] = id;
    LOG("requesting connection with %d\n", id);
    safe_call(mq_send(server_que_id, msg, MSG_SIZE, CONNECT));

    safe_call(mq_receive(client_que_id, msg, MSG_SIZE, &msg_prio));
    while(msg_prio != CONNECT){
        handle_message(msg);
        safe_call(mq_receive(client_que_id, msg, MSG_SIZE, &msg_prio));
    }
    handle_message(msg);
}

void stop(){
    char msg[MSG_SIZE];
    if(connected_with != -1) mq_close(connected_with);
    msg[0] = client_id;
    safe_call(mq_send(server_que_id, msg, MSG_SIZE, STOP));
    mq_close(server_que_id);
    exit(0);
}

void disconnect(){
    print("disconnecting...\n");
    char msg[MSG_SIZE];
    msg[0] = client_id;
    msg[1] = connected_with_id;
    safe_call(mq_send(server_que_id, msg, MSG_SIZE, DISCONNECT));

    mq_close(connected_with);
    connected_with = -1;

    safe_call(mq_receive(client_que_id, msg, MSG_SIZE, &msg_prio));
    while(msg_prio != DISCONNECT){
        handle_message(msg);
        safe_call(mq_receive(client_que_id, msg, MSG_SIZE, &msg_prio));
    }
}

void list(){
    char msg[MSG_SIZE];
    msg[0] = client_id;
    safe_call(mq_send(server_que_id, msg, MSG_SIZE, LIST));
    safe_call(mq_receive(client_que_id, msg, MSG_SIZE, &msg_prio));
    while(msg_prio != LIST){
        handle_message(msg);
        safe_call(mq_receive(client_que_id, msg, MSG_SIZE, &msg_prio));
    }

    print("My id: %d\n", client_id);
    print("%s\n", msg);
}

void register_client(){
    LOG("Registering...\n");
    char msg[MSG_SIZE];
    strcpy(msg, name);

    safe_call(mq_send(server_que_id, msg, MSG_SIZE, INIT));
    safe_call(mq_receive(client_que_id, msg, MSG_SIZE, &msg_prio));
    if (msg_prio != INIT) error("client init");

    client_id = msg[0];
    if(0 > client_id or client_id >= MAX_CLIENTS) {
        print("Failed to register\n");
        exit(-1);
        }
    LOG("Client registered at %d\n", client_id);
}

void handle_message(string msg){
    if(msg_prio == MESSAGE){
        print("message: %s\n", msg);
    }else if (msg_prio == CONNECT){
        LOG("Last connection %ld\n", (long int)last_connection);
        if(last_connection != NULL and strcmp(&msg[1], last_connection)==0) return; 
        else if (last_connection != NULL) {
            LOG("last connection >%s<\n", last_connection);
            free(last_connection); last_connection = NULL;
        }
        print("Estabilishing connection...\n");
        connected_with_id = msg[0];
        connected_with = mq_open(&msg[1], O_WRONLY | O_EXCL);
        if(connected_with < 0) print("failed to connect\n");
    }else if(msg_prio == DISCONNECT){
        print("Disconnecting...\n");
        if(last_connection != NULL and strcmp(msg, last_connection)==0) {
            free(last_connection);
            last_connection=NULL;
        }
        last_connection = new(256, c_size);
        strcpy(last_connection, msg);
        mq_close(connected_with);
        connected_with = -1;
    }else if(msg_prio == STOP_SERVER){
        stop();
    }else{
        print("Received message of type %u (ignoring)\n", msg_prio);
    }
}

void handle_input(string line){
    string command = strtok(line, " \n\t");
    if(command == NULL) return;
    LOG(">%s<\n", command);
    if(strcmp(command, "CONNECT") == 0){
        string id = strtok(NULL, " \n\t");
        if(id==NULL) {print("provide id for connection...\n"); return;}
        connect(atoi(id));
    }else if (strcmp(command, "STOP") == 0){
        stop();
    } else if (strcmp(command, "LIST") == 0){
        list();
    }else if (strcmp(command, "DISCONNECT") == 0){
        if(last_connection!=NULL) {free(last_connection); last_connection=NULL;}
        disconnect();
        LOG("Disconnected...");
    }else{ // not a command
        if(connected_with == -1){
            print("not a valid command\n");
        } else {
            // message
            LOG("sending message...\n");
            char msg[MSG_SIZE];
            strncpy(msg, line, MSG_SIZE);
            safe_call(mq_send(connected_with, msg, MSG_SIZE, MESSAGE));
        }
    }

}

void close_queue(){
    if (client_que_id != -1){
        mq_close(client_que_id);
    }
    mq_unlink(name);
}

void sigint_handler(int sig){
    stop();
}

int main(int argc, char** argv) {
    signal(SIGINT, sigint_handler);
    atexit(close_queue);
    name_me();

    server_que_id = mq_open(SERVER_NAME, O_WRONLY | O_EXCL);
    if (server_que_id < 0)    error("server mq_open");

    struct mq_attr specs;
    specs.mq_maxmsg = MAX_MESSAGES;
    specs.mq_msgsize = MSG_SIZE; 
    client_que_id = mq_open(name, O_RDONLY | O_CREAT | O_EXCL, 0666, &specs);
    if (client_que_id < 0)    error("client mq_open");
    
    register_client();
    string line = new(MSG_SIZE, c_size);
    size_t n = MSG_SIZE;
    char msg[MSG_SIZE];
    print("enter commands or press enter to process messages\n");
    struct timespec time_nonblock;
    time_nonblock.tv_nsec = 10;
    time_nonblock.tv_sec = 0;
    forewer{
        print(">");
        safe_call(getline(&line, &n, stdin))
        while(-1 != mq_timedreceive(client_que_id, msg, MSG_SIZE, &msg_prio, &time_nonblock)){
            LOG("handle message\n");
            handle_message(msg);
        }
        LOG("handle input\n");
        handle_input(line);
    }

    free(line);
}
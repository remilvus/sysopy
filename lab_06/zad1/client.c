#include "preproc.h"

int server_que_id;
int client_que_id = -1;
int client_id;
int connected_with = -1;

void connect(string id){
    MsgBuf msg;
    strcpy(msg.mtext, id);
    msg.mtype = CONNECT;
    msg.id = client_id;
    safe_call(msgsnd(server_que_id, &msg,  BUF_SIZE, 0));
    safe_call(msgrcv(client_que_id, &msg, BUF_SIZE, CONNECT, 0));
    if(msg.id == client_id){
        print("Failed to connect\n");
    } else {
        connected_with = msg.id;
        print("Connected\n");
    }
}

void stop(){
    MsgBuf msg;
    msg.mtype = STOP;
    msg.id = client_id;
    safe_call(msgsnd(server_que_id, &msg,  BUF_SIZE, 0));
    exit(0);
}

void disconnect(){
 //   print("Disconnecting...\n");
    MsgBuf msg;
    while(-1 != msgrcv(client_que_id, &msg, BUF_SIZE, CONNECT, IPC_NOWAIT)){
        LOG("ignoring connection request...\n");
    } // clear connection request
    msg.mtype = DISCONNECT;
    msg.id = client_id;
    safe_call(msgsnd(server_que_id, &msg,  BUF_SIZE, 0));
}

void list(){
    MsgBuf msg;
    msg.mtype = LIST;
    msg.id = client_id;
    safe_call(msgsnd(server_que_id, &msg,  BUF_SIZE, 0));
    safe_call(msgrcv(client_que_id, &msg, BUF_SIZE, LIST, 0));
    print("My id: %d\n", client_id);
    print("%s\n", msg.mtext);
}

void register_client(key_t key){
    LOG("Registering...\n");
    struct MsgBuf msg;
    msg.mtype = INIT;
    msg.id = client_que_id;
    msg.key = key;
    safe_call(msgsnd(server_que_id, &msg,  BUF_SIZE, 0));
    safe_call(msgrcv(client_que_id, &msg, BUF_SIZE, 0, 0));
    if(0 > msg.id or msg.id >= MAX_CLIENTS) {
        print("Failed to register\n");
        exit(-1);
        }
    client_id = msg.id;
    LOG("Client registered at %d\n", msg.id);
}

void handle_message(MsgBuf* msg){
    if(msg->mtype == MESSAGE){
        print("message: %s\n", msg->mtext);
    }else if (msg->mtype == CONNECT){
        print("Other client requested connection...\nConnecting...\n");
        connected_with = msg->id;
    }else if(msg->mtype == DISCONNECT){
        print("Disconnecting...\n");
        connected_with = -1;
        if(-1 != msgrcv(client_que_id, msg, BUF_SIZE, CONNECT, IPC_NOWAIT)){
            LOG("ignoring connection request...\n");
        } // clear connection request
    }else if(msg->mtype == STOP_SERVER){
        stop();
    }else{
        print("Received message of type %ld (ignoring)\n", msg->mtype);
    }
}

void handle_input(string line){
    string command = strtok(line, " \n\t");
    if(command == NULL) return;
    LOG(">%s<\n", command);
    if(strcmp(command, "CONNECT") == 0){
        string id = strtok(NULL, " \n\t");
        if (id==NULL){
            print("provide id...\n");
            return;
        }
        connect(id);
    }else if (strcmp(command, "STOP") == 0){
        stop();
    } else if (strcmp(command, "LIST") == 0){
        list();
    }else if (strcmp(command, "DISCONNECT") == 0){
        disconnect();
        LOG("Disconnected...");
    }else{ // not a command
        if(connected_with == -1){
            print("not a valid command\n");
        } else {
            // message
            LOG("sending message...\n");
            struct MsgBuf msg;
            msg.mtype = MESSAGE;
            strncpy(msg.mtext, line, MSG_SIZE);
            safe_call(msgsnd(connected_with, &msg,  BUF_SIZE, 0));
        }
    }

}

void close_queue(){
    if (client_que_id != -1){
        safe_call(msgctl(client_que_id, IPC_RMID, NULL));
    }
}

void sigint_handler(int sig){
    stop();
}

int main(int argc, char** argv) {
    signal(SIGINT, sigint_handler);
    atexit(close_queue);

    key_t server_key = ftok(getenv("HOME"), PROJECT_ID);
     if (server_key == -1)   error("server ftok");

    server_que_id = msgget(server_key, 0);
    if (server_que_id < 0)    error("server msgget");

    key_t clinet_key = ftok(getenv("HOME"), getpid());
    if (clinet_key == -1)   error("client ftok");

    client_que_id = msgget(clinet_key,  IPC_EXCL | IPC_CREAT | 0666); // 0666 defines permissions
    LOG("client id=%d\n", client_que_id);
    if (client_que_id < 0)    error("server msgget");
    
    register_client(clinet_key);
    string line = new(MSG_SIZE, c_size);
    size_t n = MSG_SIZE;
    MsgBuf msg;
    print("enter commands or press enter to process messages\n");
    forewer{
        print(">");
        safe_call(getline(&line, &n, stdin))
        while(-1 != msgrcv(client_que_id, &msg, BUF_SIZE, -10, IPC_NOWAIT)){
            LOG("handle message\n");
            handle_message(&msg);
        }
        LOG("handle input\n");
        handle_input(line);
    }

    free(line);
}
#include "preproc.h"

int clients_id[] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
short is_connected[] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
int mes_que_id = -1;

int get_free_idx(){
    for_i_up_to(10){
        if(clients_id[i]==-1){
            return i;
        }
    }
    return -1;
}

void fill_list(string list){
    for_i_up_to(MAX_CLIENTS){
        if(clients_id[i]!=-1){
            sprintf(list + strlen(list), "id %d is active and", i);
            if(is_connected[i] == -1){
                sprintf(list + strlen(list), " is available for connection\n");
            } else {
                sprintf(list + strlen(list), " is not available for connection\n");
            }
        }
    }
}

void disconnect(int idx1, int idx2){
    LOG("disconnext clients %d and %d\n", idx1, idx2);
    is_connected[idx1] = -1;
    is_connected[idx2] = -1;
    MsgBuf msg;
    msg.mtype = DISCONNECT;
    if(clients_id[idx1] != -1) {
        LOG("disconnecting %d\n", idx1);
        safe_call(msgsnd(clients_id[idx1], &msg, BUF_SIZE, 0));
    }
    if(clients_id[idx2] != -1) {
        LOG("disconnecting %d\n", idx2);
        safe_call(msgsnd(clients_id[idx2], &msg, BUF_SIZE, 0));
    }
}

void handle_message(MsgBuf msg){
    LOG("handling message... TYPE=%ld\n", msg.mtype);
    switch (msg.mtype)
    {
    case STOP:;
        print("client %d stopped...\n", msg.id);
        clients_id[msg.id] = -1;
        if(is_connected[msg.id] != -1) disconnect(msg.id, is_connected[msg.id]);
        break;

    case DISCONNECT:;
        disconnect(msg.id, is_connected[msg.id]);
        break;

    case LIST:;
        print("list request...\n");
        MsgBuf list_m;
        memset(list_m.mtext, 0, 1);
        fill_list(list_m.mtext);
        list_m.mtype = LIST;
        safe_call(msgsnd(clients_id[msg.id], &list_m, BUF_SIZE, 0));
        break;

    case CONNECT:;
        int id = atoi(msg.mtext);
        if(0 <= id and id < MAX_CLIENTS and clients_id[id] != -1 and id != msg.id and
                -1 == is_connected[id] and -1 == is_connected[msg.id]){
            LOG("Connection can be estabilished\n");
            is_connected[id] = msg.id;
            is_connected[msg.id] = id;
            memset(msg.mtext, 0, MSG_SIZE);
            strcpy(msg.mtext, "sucess");
            msg.mtype = CONNECT;
            int id1 = clients_id[msg.id];
            int id2 = clients_id[id];
            msg.id = id1;
            safe_call(msgsnd(id2, &msg, BUF_SIZE, 0));
            msg.id = id2;
            safe_call(msgsnd(id1, &msg, BUF_SIZE, 0));
        }else{
            LOG("Connection can't be estabilished\n");
            LOG("%d\n", msg.id);
            memset(msg.mtext, 0, MSG_SIZE);
            strcpy(msg.mtext, "failed");
            msg.mtype = CONNECT;
            safe_call(msgsnd(clients_id[msg.id], &msg, BUF_SIZE, 0));
        }
        break;

    case INIT:;
        print("new client...\n");
        int client_idx = get_free_idx();
        LOG("idx for client = %d\n", client_idx);
        if(client_idx != -1){
            clients_id[client_idx] = msg.id;
            is_connected[client_idx] = -1;
        }
        MsgBuf return_info;
        return_info.id = client_idx;
        return_info.mtype = 1;
        LOG("returning info to client...\n");
        safe_call(msgsnd(msg.id, &return_info, BUF_SIZE, 0));
        LOG("INIT ended sucessfully\n");
        break;
    
    default:
        break;
    }
}

void close_queue(){
    if (mes_que_id != -1){
        safe_call(msgctl(mes_que_id, IPC_RMID, NULL));
    }
}

void stop_clients(){
    print("waiting for all clients to stop...\n");
    MsgBuf msg;
    msg.mtype = STOP_SERVER;
    int count = 0;
    for_i_up_to(MAX_CLIENTS){
        if(clients_id[i] != -1){
            msgsnd(clients_id[i], &msg, BUF_SIZE, 0);
            count ++;
        }
    }
    while(count > 0){
        safe_call(msgrcv(mes_que_id, &msg, BUF_SIZE, STOP, 0));
        count--;
    }
    print("all clients stopped...\n");
}

void sigint_handler(int sig){
    stop_clients();
    exit(0);
}

int main(int argc, char** argv) {
    signal(SIGINT, sigint_handler);
    atexit(close_queue);

    key_t key = ftok(getenv("HOME"), PROJECT_ID);
    if (key == -1)   error("server ftok");

    mes_que_id = msgget(key, IPC_CREAT | IPC_EXCL | 0666); // 0666 defines permissions ;IPC_CREAT | IPC_EXCL 
    if (mes_que_id < 0)     error("server msgge  t");

    MsgBuf msg;
    print("server is working...\n");
    forewer{
        safe_call(msgrcv(mes_que_id, &msg, BUF_SIZE, -10, 0));
        handle_message(msg);
    }

}
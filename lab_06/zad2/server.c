#include "preproc.h"

mqd_t adress[] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
string client_name[MAX_CLIENTS];
mqd_t is_connected[] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
mqd_t mes_que_id = -1;
char name[NAME_SIZE];
unsigned int msg_prio;

void name_me(){
    memset(name, 0, NAME_SIZE);
    sprintf(name, "%s", SERVER_NAME);
}


int get_free_idx(){
    for_i_up_to(MAX_CLIENTS){
        if(adress[i]==-1){
            return i;
        }
    }
    return -1;
}

void fill_list(string list){
    for_i_up_to(MAX_CLIENTS){
        if(adress[i]!=-1){
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
    char msg[MSG_SIZE];
    if(adress[idx1] != -1) {
        LOG("disconnecting %d\n", idx1);
        strcpy(msg, client_name[idx2]);
        safe_call(mq_send(adress[idx1], msg, MSG_SIZE, DISCONNECT));

    }
    if(adress[idx2] != -1) {
        LOG("disconnecting %d\n", idx2);
        strcpy(msg, client_name[idx1]);
        safe_call(mq_send(adress[idx2], msg, MSG_SIZE, DISCONNECT));
    }
}


void handle_message(string msg){
    LOG("handling message... TYPE=%u\n", msg_prio);
    int client_id = msg[0];
    switch (msg_prio)
    {
    case STOP:;
        print("client %d stopped...\n", client_id);
        safe_call(mq_close(adress[client_id]));
        adress[client_id] = -1;
        if(is_connected[client_id] != -1) disconnect(client_id, is_connected[client_id]);
        break;

    case DISCONNECT:;
        disconnect(client_id, is_connected[client_id]);
        break;

    case LIST:;
        print("list request...\n");
        char list[MSG_SIZE];
        memset(list, 0, 1);
        fill_list(list);
        safe_call(mq_send(adress[client_id], list, MSG_SIZE, LIST));
        LOG("list has been sent\n");
        break;

    case CONNECT:;
        print("connection request\n");
        int id = msg[1];
        if(0 <= id and id < MAX_CLIENTS and adress[id] != -1 and id != client_id and
                -1 == is_connected[id] and -1 == is_connected[client_id]){
            LOG("Connection can be estabilished\n");
            is_connected[id] = client_id;
            is_connected[client_id] = id;
            mqd_t id1 = adress[client_id];
            mqd_t id2 = adress[id];
            msg[0] = client_id;
            strcpy(&msg[1], client_name[client_id]);
            safe_call(mq_send(id2, msg, MSG_SIZE, CONNECT));
            msg[0] = id;
            strcpy(&msg[1], client_name[id]);
            safe_call(mq_send(id1, msg, MSG_SIZE, CONNECT));
        }else{
            LOG("Connection can't be estabilished\n");
            LOG("%d\n", client_id);
            msg[0] = -1;
            safe_call(mq_send(adress[client_id], msg, MSG_SIZE, CONNECT));
        }
        break;

    case INIT:;
        print("new client...\n");
        int client_id = get_free_idx();
        LOG("idx for client = %d\n", client_id);

        mqd_t client_adress = mq_open(msg, O_WRONLY);
        if (client_adress==-1) error("failed to init client");

        if(client_id != -1){
            adress[client_id] = client_adress;
            strcpy(client_name[client_id], msg);
            is_connected[client_id] = -1;
        }

        msg[0] = client_id;
        LOG("returning info to client...\n");
        safe_call(mq_send(client_adress, msg, MSG_SIZE, INIT));
        LOG("INIT ended sucessfully\n");
        break;
    
    default:
        break;
    }
}

void close_queue(){
    if (mes_que_id != -1){
        safe_call(mq_close(mes_que_id));
    }
    mq_unlink(name);
}

void stop_clients(){
    print("waiting for all clients to stop...\n");
    char msg[MSG_SIZE];
    msg[0] = STOP_SERVER;
    int count = 0;
    for_i_up_to(MAX_CLIENTS){
        if(adress[i] != -1){
            safe_call(mq_send(adress[i], msg, MSG_SIZE, STOP_SERVER));
            mq_close(adress[i]);
            count ++;
        }
    }
    while(count > 0){
        safe_call(mq_receive(mes_que_id, msg, MSG_SIZE, &msg_prio));
        if(msg_prio==STOP) count--;
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
    name_me();

    for_i_up_to(MAX_CLIENTS){
        client_name[i] = new(256, c_size);
    }

    struct mq_attr specs;
    specs.mq_maxmsg = MAX_MESSAGES;
    specs.mq_msgsize = MSG_SIZE; 
    LOG("server name %s\n", name);
    mes_que_id = mq_open(name, O_RDONLY | O_CREAT | O_EXCL, 0666, &specs);
    if (mes_que_id < 0)     error("server mq_open");

    char msg[MSG_SIZE];
    print("server is working...\n");
    forewer{
        safe_call(mq_receive(mes_que_id, msg, MSG_SIZE, &msg_prio));
        handle_message(msg);
    }

}
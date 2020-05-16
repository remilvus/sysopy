#include "preproc.h"

int server_socket_fd;
string myname;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
in_port_t inet_port;
string server_IP;
int input = 0;
int connection_type;
string local_socket_path_name;

void sigint_handler(){
    printf("SIGINT\n");
    // error("client name too long\n");
    exit(EXIT_SUCCESS);
}

void check_msg_res(int res){
    if(res==0){ // connection failed
        print("connection was closed\n");
        exit(0);
    } else if(res < 0){
        error("message error\n");
    }
}

void server_connect(){
    if(connection_type == LOCAL){
        struct sockaddr_un sa;
        strcpy(sa.sun_path, local_socket_path_name);
        sa.sun_family = AF_LOCAL;

        if((server_socket_fd = socket(AF_LOCAL, SOCKET_PROTOCOL, 0)) == -1)
            error("socket");

        if(connect(server_socket_fd, (struct sockaddr*) &sa, sizeof(sa)) == -1)
            error("connect local");
    } else if(connection_type == INET){
        struct sockaddr_in inet;
        inet.sin_family = AF_INET;
        inet.sin_addr.s_addr = inet_addr(server_IP);
        inet.sin_port = htons(inet_port);
        LOG("port %d\n", inet_port);
        LOG("ip %s\n", server_IP);

        if((server_socket_fd = socket(AF_INET, SOCKET_PROTOCOL, 0)) == -1)
            error("socket inet");

        if(connect(server_socket_fd, (struct sockaddr*) &inet, sizeof(inet)) == -1)
            error("connect inet");
    } else {
        error("wont happen\n");
    }
}

bool handle_message(string msg){
    if(strcmp(msg, "ping")==0){
        check_msg_res(send_message(server_socket_fd, "pong"));
    }else if(strlen(msg) == 9){ // received game
        char board[12];
        for(int i=0; i<3;i++) board[i] = msg[i];
        board[3] = '\n';
        for(int i=4; i<7;i++) board[i] = msg[i-1];
        board[7] = '\n';
        for(int i=8; i<11;i++) board[i] = msg[i-2];
        board[11] = '\n';

        print("Board state:\n%s\nMake a move (1-9)\n>", board);
        return true;
    } else if(strcmp("bad move", msg)==0){
        print("wrong move\n>");
        return true;
    } else{
        print("Server message: %s\n>", msg);
    }
    return false;
}

void close_client(){
    LOG("close client\n");
  //  check_msg_res(send_message(server_socket_fd, "end"));
//    shutdown(server_socket_fd, SHUT_RDWR);
    LOG("close\n");
   // safe_call(close(server_socket_fd));
    // todo; nah
}

void* message_manager(void* arg){
    char buffer[MESSAGE_SIZE];
    forewer{
        check_msg_res(receive_message(server_socket_fd, buffer));
        bool ask_input = handle_message(buffer);
        if(ask_input){
            LOG("asking for input\n");
            
            pthread_cond_signal(&cond); 
        }
    }
}

int main(int argc, char** argv) {
    if (argc != 4) error("wrong number of arguments\n");
    if (strlen(argv[1]) >= MESSAGE_SIZE) error("client name too long\n");
    if (strcmp(argv[2], "inet") == 0) connection_type = INET;
    else if(strcmp(argv[2], "local") == 0) connection_type = LOCAL;
    else error("Bad connection type\n");
    if(connection_type == LOCAL){
        local_socket_path_name = argv[3];
    } else {
        strtok(argv[3], ":");
        char* ip_end = strtok(NULL, ":");
        if(ip_end == NULL) error("Bad server adress\n");
        ip_end -= c_size;
        *ip_end = 0;
        ip_end += c_size;
        LOG("IP=%s | port=%s\n", argv[3], ip_end);
        server_IP = argv[3];
        inet_port = (in_port_t) atoi(ip_end);
        // todo take inet addres
    }
    if (atexit(close_client) == -1) error("atexit");
    myname = argv[1];

    
    // handler for `CTRL + C`
    if ((signal(SIGINT, &sigint_handler)) < 0) error("signal");

    server_connect();

    // register
    char buffer[MESSAGE_SIZE];
    check_msg_res(send_message(server_socket_fd, myname));
    check_msg_res(receive_message(server_socket_fd, buffer));
    if(strcmp(buffer, "ok") != 0) error("unexpected message from server\n");
    LOG("first server msg: %s\n", buffer);

    pthread_mutex_lock(&mutex);
    pthread_t message_maganer_tid;
    pthread_create(&message_maganer_tid, NULL, message_manager, NULL);


    forewer{
        pthread_cond_wait(&cond, &mutex); // unlock for input
        print("take action (1-9: make move | 0: end game)\n>");
        scanf("%s", buffer); 
        input = buffer[0];
        if(input == '0') exit(0);
        buffer[1] = 0;
        LOG("message manager sending: >%d<\n", buffer[0]);
        check_msg_res(send_message(server_socket_fd, buffer));
    }

    pthread_join(message_maganer_tid, NULL);

}
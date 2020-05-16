#include "preproc.h"

int server_socket_fd;
string myname;
char myid;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
in_port_t inet_port;
in_port_t my_inet_port;
struct sockaddr_un sa_local;
string server_IP;
struct sockaddr_in sa_inet;
int input = 0;
int connection_type;
string local_socket_path_name;


int send_message(int socket_fd, const char message[MESSAGE_SIZE]) {
    char buffer[MESSAGE_SIZE];
    buffer[0] = GAME_MSG;
    buffer[1] = myid; 
    strcpy(&buffer[2], message);
    if(connection_type == LOCAL) return sendto(socket_fd, buffer, MESSAGE_SIZE, 0, (struct sockaddr*) &sa_local, sizeof(sa_local));
    else return sendto(socket_fd, buffer, MESSAGE_SIZE, 0,  (struct sockaddr*) &inet_port, sizeof(inet_port)); // inet
}

int receive_message(string buffer) {
    socklen_t size = sizeof(sa_local);
    if(connection_type == LOCAL) return recvfrom(server_socket_fd, buffer, MESSAGE_SIZE, 0, (struct sockaddr*) &sa_local, &size);
    else return recvfrom(server_socket_fd, buffer, MESSAGE_SIZE, 0, (struct sockaddr*)&sa_inet, &size); // inet
}

void sigint_handler(){
    printf("SIGINT\n");
    // error("client name too long\n");
    exit(EXIT_SUCCESS);
}

void check_msg_res(int res){
    if(res==0){ // connection failed
        print("shutdown was preformed by peer\n");
        exit(0);
    } else if(res < 0){
        error("message error\n");
    }
}

void server_connect(){
    if(connection_type == LOCAL){
        sa_local.sun_family = AF_LOCAL;

        // create server socket
        if((server_socket_fd = socket(AF_LOCAL, SOCK_DGRAM, 0)) == -1)
            error("socket");

        strcpy(sa_local.sun_path, myname);
        if ((bind(server_socket_fd, (struct sockaddr*) &sa_local, sizeof(sa_local))) == -1)  
            error("cannot bind local socket");

        strcpy(sa_local.sun_path, local_socket_path_name);
        connect(server_socket_fd, (struct sockaddr*) &sa_local, sizeof(sa_local));
        perror(":c");
        //  if ((connect(server_socket_fd, (struct sockaddr*) &sa_local, sizeof(sa_local))) == -1)  
        //     error("cannot bind local socket");
        strcpy(sa_local.sun_path, local_socket_path_name);

    } else if(connection_type == INET){
        sa_inet.sin_family = AF_INET;
        sa_inet.sin_addr.s_addr = inet_addr(server_IP);
        
        LOG("port %d\n", sa_inet.sin_port );
        LOG("ip %s\n", server_IP);

        if((server_socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
            error("socket inet");
        my_inet_port = inet_port + 1;
        sa_inet.sin_port = htons(my_inet_port);
        while ((bind(server_socket_fd, (struct sockaddr*) &sa_inet, sizeof(sa_inet))) == -1)  {
            my_inet_port = my_inet_port + 1;
            sa_inet.sin_port = htons(my_inet_port);
        }

        sa_inet.sin_port = htons(inet_port);
        connect(server_socket_fd, (struct sockaddr*) &sa_inet, sizeof(sa_inet));
        //  if ((connect(server_socket_fd, (struct sockaddr*) &sa_inet, sizeof(sa_inet))) == -1)  
        //     error("cannot bind inet socket");

    } else {
        error("wont happen\n");
    }
}

bool handle_message(string msg){
    if(strcmp(msg, "ping")==0){
        check_msg_res(send_message(server_socket_fd, "pong"));
    }else if(strcmp(msg, "die")==0){
        print("Closing client...\n");
        exit(0);
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
    send_message(server_socket_fd, "end");
    unlink(myname);
    close(server_socket_fd);
}

void* message_manager(void* arg){
    char buffer[MESSAGE_SIZE];
    forewer{
        LOG("waiting for msg\n");
        check_msg_res(receive_message(buffer));
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

    }
    if (atexit(close_client) == -1) error("atexit");
    myname = argv[1];

    
    // handler for `CTRL + C`
    if ((signal(SIGINT, &sigint_handler)) < 0) error("signal");

    server_connect();

    // register
    
    char buffer[MESSAGE_SIZE];
    buffer[0] = REGISTER_MSG;
    if(connection_type == LOCAL) {
        buffer[1] = LOCAL;
        sprintf(&buffer[MSG_START], "%s|%s", myname, myname);
    }
    else {
        buffer[1] = INET;
        LOG("addr type = %d\n", ADDR_TYPE(buffer));
        sprintf(&buffer[MSG_START], "%s|%d|%s", server_IP, my_inet_port,  myname);
    }

    LOG("registering...\n");
    if(connection_type == LOCAL){
         sendto(server_socket_fd, buffer, MESSAGE_SIZE, 0, (struct sockaddr*) &sa_local, sizeof(sa_local));
         perror("lcl msg\n");
         LOG("send local msg\n");
    }
    else {
        LOG("sending by inet...\n");
        LOG("%d, %s, %d\n", server_socket_fd, buffer, sa_inet.sin_port);
        LOG("addr type = %d\n", ADDR_TYPE(buffer));
        sendto(server_socket_fd, buffer, MESSAGE_SIZE, 0,  (struct sockaddr*) &sa_inet, sizeof(sa_inet));
        perror("ehh\n");
    }

    check_msg_res(receive_message(buffer));
    myid = buffer[2];
    buffer[2] = 0;
    if(strcmp(buffer, "ok") != 0){buffer[2] = myid; LOG("msg=%s\n", buffer); error("unexpected message from server\n");}
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
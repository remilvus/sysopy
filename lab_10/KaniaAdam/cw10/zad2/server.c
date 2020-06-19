#include "preproc.h"

string local_socket_path_name = NULL;
int local_server_socket_fd = -1;
int inet_server_socket_fd = -1;
struct pollfd sockets[2];
struct sockaddr_un so_addr;
clients_t clients;
int waiting_game_id = -1;
in_port_t inet_port;
pthread_mutex_t mutex_ping = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_ping = PTHREAD_COND_INITIALIZER;


void* connection_manager(void*);
void* ping_manager(void*);
void* game_manager(void*);

int send_message(int id, const char message[MESSAGE_SIZE]) {
    if(clients.connection_type[id] == LOCAL) 
        return sendto(local_server_socket_fd, message, MESSAGE_SIZE, 0, (struct sockaddr*) &clients.so_addr_un[id], sizeof(struct sockaddr_un));
    else if(clients.connection_type[id] == INET) 
        return sendto(inet_server_socket_fd, message, MESSAGE_SIZE, 0, (struct sockaddr*) &clients.so_addr_in[id], sizeof(struct sockaddr_in)); // inet
    else error("Bad connection type | send_message\n");
    return -1;
}

int receive_message_from(int id, string buffer, bool block) {
    socklen_t size = sizeof(struct sockaddr_un);
    int flags = MSG_DONTWAIT;
    if(block) flags = 0;
    if(clients.connection_type[id] == LOCAL) 
        return recvfrom(local_server_socket_fd, buffer, MESSAGE_SIZE, flags, (struct sockaddr*) &clients.so_addr_un[id], &size);
    else if(clients.connection_type[id] == INET) 
        return recvfrom(inet_server_socket_fd, buffer, MESSAGE_SIZE, flags, (struct sockaddr*) &clients.so_addr_in[id],  &size); // inet
    else error("Bad connection type | receive_message_from\n");
    return -1;
}

void sigint_handler(){
    printf("SIGINT\n");
    exit(EXIT_SUCCESS);
}

bool check_existance(string name){
    for_i_up_to(SERVER_CLIENTS_LIMIT){
        if(strcmp(clients.name[i], name) == 0){
            return true;
        }
    }
    return false;
}

void exit_fun(){
    if(inet_server_socket_fd != -1) {
        close(inet_server_socket_fd);
    }
    if(local_server_socket_fd != -1) {
        close(local_server_socket_fd);
        unlink(local_socket_path_name);
    }

}

void init_game(int client_id){
    pthread_mutex_lock(&clients.mutex_game);
    if(waiting_game_id == -1){// start new game
        int res = send_message(client_id, "waiting for second player...\n");
        LOG("send waiting msg %d\n", res);
        if(res>0){
            clients.games[client_id].second_player = -1;
            waiting_game_id = client_id;
        }
    } else { // game exists
        int first_player;
        LOG("starting game between %d & %d\n", waiting_game_id, client_id);
        if(random() % 2 == 0) first_player = waiting_game_id; else first_player = client_id;
        clients.games[waiting_game_id].second_player = client_id;
        clients.games[client_id].second_player = waiting_game_id;
        clients.games[waiting_game_id].type = 'x';
        send_message(waiting_game_id, "You are `x`");
        clients.games[waiting_game_id].code = 1;
        clients.games[client_id].type = 'o';
        send_message(client_id, "You are `o`");
        clients.games[client_id].code = 0;
        //int res = 
        for_i_up_to(10) { // clear boards
            clients.games[waiting_game_id].game_state[i] = -1;
            clients.games[client_id].game_state[i] = -1;
        }
        send_message(first_player, "_________"); // sends empty board
        waiting_game_id = -1;
    }
    pthread_mutex_unlock(&clients.mutex_game);
}

void remove_client(int id){
    pthread_mutex_lock(&clients.mutex_client);
 //   int client_fd = clients.fd[id];
    clients.fd[id] = -1;
    clients.name[id][0] = 0;
    if(waiting_game_id == id) waiting_game_id = -1;
    clients.connection_type[id] = -1;
    clients.count--;
    pthread_mutex_unlock(&clients.mutex_client);
}

short check_win(int* board){
    if(board[5] != -1){
        if(board[5] == board[7] and board[5] == board[3]) return board[5];
        if(board[5] == board[1] and board[5] == board[9]) return board[5];
    }
    for_i_up_to(3){
        // check rows
        int start = (3*i)+1;
        if(board[start] != -1){
            if(board[start] == board[start+1] and board[start] == board[start+2]) return board[start];
        }
        // check columns
        start = i+1;
        if(board[start] != -1){
            if(board[start] == board[start+3] and board[start] == board[start+6]) return board[start];
        }
    }
    for(int i=1; i<11; i++) if(board[i]==-1) return -1;
    return 2;
}

void board_to_string(int* board, string buffer){
    // todo
    for_i_up_to(10){
        if (board[i]==1) buffer[i-1] = 'x';
        else if (board[i]==0) buffer[i-1] = 'o';
        else buffer[i-1] = '_';
    }
    buffer[9] = 0;
}

void end_game(int winner_code, int id){
    int enemy = clients.games[id].second_player;
    if(winner_code != 2){
        int winner = enemy; 
        int loser = id;
        if(clients.games[id].code == winner_code){
            winner = id;
            loser = enemy;
        }
        send_message(winner, "you have won\n");
        send_message(loser, "you have lost\n");
    } else{
        send_message(enemy, "draw\n");
        send_message(id, "draw\n");
    }

    send_message(enemy, "die");
    send_message(id, "die");
    remove_client(enemy);
    remove_client(id);
}

void abandon_game(int abandoner){
        print("Game was abandoned by %d\n", abandoner);
        int enemy = clients.games[abandoner].second_player;
        clients.games[abandoner].second_player = -1;
        if(waiting_game_id == abandoner) waiting_game_id = -1;
        if(enemy != -1) {
            send_message(enemy, "game was abandoned\n");
            clients.games[enemy].second_player = -1;
            init_game(enemy);
        }
}

void advance_game(char* msg, int player_id){
    LOG("advance game id=%d|msg=%s\n", player_id, msg);
    if(strcmp(msg, "end")==0){ // end game & remove client
        LOG("game was abandoned by %d\n", player_id);
        abandon_game(player_id);
        remove_client(player_id);
        return;
    }
    int move = atoi(msg);
    LOG("processing move %d\n", move);
    if(0 < move and move < 10){ // move is on board
        pthread_mutex_lock(&clients.mutex_game);
        if(clients.games[player_id].game_state[move] == -1) {// valid move
            clients.games[player_id].game_state[move] = clients.games[player_id].code;
            int enemy = clients.games[player_id].second_player;
            clients.games[enemy].game_state[move] = clients.games[player_id].code;
            int* board = clients.games[enemy].game_state;
            int winner = check_win(board);
            if (winner == 0 or winner == 1 or winner == 2){
                end_game(winner, player_id);
            } else {
                char buffer[MESSAGE_SIZE];
                board_to_string(board, buffer);
                print("sending next board state to %d\n", enemy);
                send_message(enemy, buffer);
            }
        } else {
             print("sending bad move message\n");
            send_message(player_id, "bad move");
        }
        pthread_mutex_unlock(&clients.mutex_game);
    } else{
        print("sending bad move message\n");
        send_message(player_id, "bad move");
    }
}


void start_server(){
    // local socket setup
    printf("openening local socket | path: %s\n", local_socket_path_name);

    strcpy(so_addr.sun_path, local_socket_path_name);
    so_addr.sun_family = AF_LOCAL;

    // handler for `CTRL + C`
    if ((signal(SIGINT, &sigint_handler)) < 0) error("signal");
    
    LOG("create & bind socket\n");
    if ((local_server_socket_fd = socket(AF_LOCAL, SOCK_DGRAM, 0)) == -1)               error("cannot create local socket");
    if ((bind(local_server_socket_fd, (struct sockaddr*) &so_addr, sizeof(so_addr))) == -1)  error("cannot bind local socket");

    // INET socket setup:
    struct hostent * host = gethostbyname("localhost");
    struct in_addr host_address = *(struct in_addr*) host->h_addr;

    struct sockaddr_in sa_inet;
    sa_inet.sin_family = AF_INET;
    sa_inet.sin_addr.s_addr = host_address.s_addr;
    sa_inet.sin_port = htons(inet_port);

    if ((inet_server_socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)               error("cannot create inet socket");
    if ((bind(inet_server_socket_fd, (struct sockaddr*) &sa_inet, sizeof(sa_inet))) == -1) error("cannot bind inet socket");
 
    printf("INET socket address: %s:%d\n", inet_ntoa(host_address), inet_port);

    sockets[0].fd = local_server_socket_fd;
    sockets[1].fd = inet_server_socket_fd;
    sockets[0].events = sockets[1].events = POLLIN;
}

void init(){
    srand(time(NULL));
    clients.count = 0;
    for_i_up_to(SERVER_CLIENTS_LIMIT){
        clients.fd[i] = -1;
        clients.name[i] = new(MESSAGE_SIZE, char);
        clients.connection_type[i] = -1;
    }
    pthread_mutex_init(&clients.mutex_client, NULL);
    pthread_mutex_init(&clients.mutex_game, NULL);
}

int main(int argc, char** argv) {
    if (atexit(exit_fun) == -1) error("atexit");
    if (argc != 3) error("wrong number of arguments\n");
    inet_port = (in_port_t) atoi(argv[1]);
    local_socket_path_name = argv[2];

    init();

    start_server();

    LOG("create managers\n");
    pthread_t ping_manager_tid;
    pthread_create(&ping_manager_tid, NULL, ping_manager, NULL);

    pthread_t input_manager_tid;
    pthread_create(&input_manager_tid, NULL, game_manager, NULL);

    pthread_join(ping_manager_tid, NULL);
    pthread_join(input_manager_tid, NULL);

}

void* ping_manager(void* arg){
    char buffer[MESSAGE_SIZE];
    forewer{
        pthread_cond_wait(&cond_ping, &mutex_ping);
        for_i_up_to(SERVER_CLIENTS_LIMIT){
            
            if(clients.fd[i] != -1){
                LOG("send ping\n");
                int res = send_message(i, "ping");
                if(res == -1)  {
                    print("Ping failed (id=%d)\n", i);
                    abandon_game(i);
                    remove_client(i); 
                    continue;
                }
                usleep(PING_TIME);
                res = receive_message_from(i, buffer, false);
                if (res <= 0 or strcmp(&buffer[2], "pong")!=0){
                    LOG("ping failed (id=%d | fd=%d) | buffer: %s | res=%d\n", i,clients.fd[i], buffer, res);
                    print("Ping failed (id=%d)\n", i);
                    abandon_game(i);
                    remove_client(i);
                }
            }
        }
        pthread_cond_broadcast(&cond_ping);
    }

    return (void*) 0;
}

void* game_manager(void* arg){
    pthread_mutex_lock(&mutex_ping);
    char buffer[MESSAGE_SIZE];

    forewer{
        for_i_up_to(2){sockets[i].revents = 0;}
        poll(sockets, 2, -1);
        int socket = -1;
        for_i_up_to(2) if(sockets[i].revents & POLLIN) socket = sockets[i].fd;
        if(socket == -1) continue;
        ssize_t msg_size = recv(socket, buffer, MESSAGE_SIZE, 0);
        if(msg_size == -1) {LOG("received message, but error occured\n");}
        else if(msg_size == 0) continue;
        LOG("received message %s\n", buffer);
        if(TYPE(buffer) is REGISTER_MSG){
            LOG("registering...\n");
            pthread_mutex_lock(&clients.mutex_client);
            int free_id = -1;
            for_i_up_to(SERVER_CLIENTS_LIMIT){
                if(clients.fd[i] == -1) {free_id = i; break;}
            }

            if(free_id == -1){ // no free slot for client

                strcpy(buffer, "clients limit reached");
                socklen_t size = sizeof(struct sockaddr);
                LOG("sending reply 'clients limit reached'...\n");
                if(ADDR_TYPE(buffer) is LOCAL){ 
                    struct sockaddr_un soard;
                    char* local_path = strtok(&buffer[MSG_START], "|");
                    char* name_start = strtok(NULL, "|");
                    name_start -= c_size; *name_start = 0; name_start += c_size;
                    soard.sun_family = AF_LOCAL;
                    strcpy(soard.sun_path, local_path);
                    sendto(local_server_socket_fd, buffer, MESSAGE_SIZE, 0, (struct sockaddr*) &soard, size);
                } else if(ADDR_TYPE(buffer) is INET) {
                    struct sockaddr_in soard;
                    
                    char* client_ip = strtok(&buffer[MSG_START], "|");
                    char* ip_end = strtok(NULL, "|");
                    ip_end -= c_size; *ip_end = 0; ip_end += c_size;
                    char* name_start = strtok(NULL, "|");
                    name_start -= c_size; *name_start = 0; name_start += c_size;
                    LOG("inet adress received: %s:%s | name=%s\n", client_ip, ip_end, name_start);
                    soard.sin_family = AF_INET;
                    soard.sin_addr.s_addr = inet_addr(client_ip);
                    soard.sin_port = htons((in_port_t) atoi(ip_end));

                    sendto(inet_server_socket_fd, buffer, MESSAGE_SIZE, 0, (struct sockaddr*) &soard,  size); 
                }
              //  else error("Bad connection type | receive_message_from\n");
            
                pthread_mutex_unlock(&clients.mutex_client);
                continue;
            }

            // register client
            
            if(ADDR_TYPE(buffer) is LOCAL){
                LOG("registring local...\n");
                char* local_path = strtok(&buffer[MSG_START], "|");
                char* name_start = strtok(NULL, "|");
                name_start -= c_size; *name_start = 0; name_start += c_size;
                //todo check name
                if(check_existance(name_start)){
                    struct sockaddr_un soard;

                    soard.sun_family = AF_LOCAL;
                    strcpy(soard.sun_path, local_path);

                    sendto(inet_server_socket_fd, "name taken", MESSAGE_SIZE, 0, (struct sockaddr*) &soard,  sizeof(struct sockaddr_un)); // inet
                    pthread_mutex_unlock(&clients.mutex_client);
                    continue;
                }
                strcpy(clients.name[free_id], name_start);
                clients.connection_type[free_id] = LOCAL;
                clients.so_addr_un[free_id].sun_family = AF_LOCAL;
                strcpy(clients.so_addr_un[free_id].sun_path, local_path);

            } else if (ADDR_TYPE(buffer) is INET){
                LOG("addr inet\n");
                char* client_ip = strtok(&buffer[MSG_START], "|");
                char* ip_end = strtok(NULL, "|");
                ip_end -= c_size; *ip_end = 0; ip_end += c_size;
                char* name_start = strtok(NULL, "|");
                name_start -= c_size; *name_start = 0; name_start += c_size;
                LOG("inet adress received: %s:%s | name=%s\n", client_ip, ip_end, name_start);

                if(check_existance(name_start)){
                    LOG("name taken\n");
                    struct sockaddr_in soard;

                    soard.sin_family = AF_INET;
                    soard.sin_addr.s_addr = inet_addr(client_ip);
                    soard.sin_port = htons((in_port_t) atoi(ip_end));

                    sendto(inet_server_socket_fd, "name taken", MESSAGE_SIZE, 0, (struct sockaddr*) &soard,  sizeof(struct sockaddr_in)); // inet
                    pthread_mutex_unlock(&clients.mutex_client);
                    continue;
                }

                clients.connection_type[free_id] = INET;
                strcpy(clients.name[free_id], name_start);

                clients.so_addr_in[free_id].sin_family = AF_INET;
                clients.so_addr_in[free_id].sin_addr.s_addr = inet_addr(client_ip);
                clients.so_addr_in[free_id].sin_port = htons((in_port_t) atoi(ip_end));

            } else {
                LOG("addr type = %d\n", ADDR_TYPE(buffer));
                error("wrong adres type\n");
            }
            clients.fd[free_id] = socket;

            // send ok msg
           buffer[0]='o'; buffer[1]='k'; buffer[2]=free_id; buffer[3]=0;
           send_message(free_id, buffer);

           pthread_mutex_unlock(&clients.mutex_client);
           init_game(free_id);

        } else if(TYPE(buffer) is GAME_MSG){
            advance_game(&buffer[MSG_START], WHO(buffer)-1);

        }
        pthread_cond_broadcast(&cond_ping);
        pthread_cond_wait(&cond_ping, &mutex_ping);
    }

    return (void*) 0;
}
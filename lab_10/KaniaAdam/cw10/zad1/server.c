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
        if ((close(inet_server_socket_fd)) == -1)       error("closing inet socket");
    }
    if(local_server_socket_fd != -1) {
        if ((close(local_server_socket_fd)) == -1)       error("closing local socket");
        if ((unlink(local_socket_path_name)) == -1)      error("unlinking local socket file");
    }

}

int safe_send(string msg, int id){
    char buffer[MESSAGE_SIZE];
    strcpy(buffer, msg);
    pthread_mutex_lock(&clients.mutex_client);
    if(fcntl(clients.fd[id], F_GETFD)==-1) {LOG("sending msg failed\n"); pthread_mutex_unlock(&clients.mutex_client); return -1;}
    int res = send(clients.fd[id], buffer, MESSAGE_SIZE, MSG_NOSIGNAL);
    if(res > 0) res = 0;
    pthread_mutex_unlock(&clients.mutex_client);
    return res;
}

void init_game(int client_id){
    pthread_mutex_lock(&clients.mutex_game);
    if(waiting_game_id == -1){// start new game
        LOG("send waiting msg\n");
        print("%d waiting for second player\n", client_id);
        int res = safe_send("waiting for second player...\n", client_id);
        if(res==0){
            clients.games[client_id].second_player = -1;
            waiting_game_id = client_id;
        }
    } else { // game exists
        int first_player;
        print("Starting game between %d & %d\n", waiting_game_id, client_id);
        if(random() % 2 == 0) first_player = waiting_game_id; else first_player = client_id;
        clients.games[waiting_game_id].second_player = client_id;
        clients.games[client_id].second_player = waiting_game_id;
        clients.games[waiting_game_id].type = 'x';
        safe_send("You are `x`", waiting_game_id);
        clients.games[waiting_game_id].code = 1;
        clients.games[client_id].type = 'o';
        safe_send("You are `o`", client_id);
        clients.games[client_id].code = 0;
        //int res = 
        for_i_up_to(10) { // clear boards
            clients.games[waiting_game_id].game_state[i] = -1;
            clients.games[client_id].game_state[i] = -1;
        }
        safe_send("_________", first_player); // sends empty board
        waiting_game_id = -1;
    }
    pthread_mutex_unlock(&clients.mutex_game);
}

void remove_client(int id){
    pthread_mutex_lock(&clients.mutex_client);
    int client_fd = clients.fd[id];
    clients.fd[id] = -1;
    clients.name[id][0] = 0;
    if(waiting_game_id == id) waiting_game_id = -1;
    shutdown(client_fd, SHUT_RDWR);
    close(client_fd);

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
    print("Game has ended (%d vs. %d)", enemy, id);
    if(winner_code != 2){
        int winner = enemy; 
        int loser = id;
        if(clients.games[id].code == winner_code){
            winner = id;
            loser = enemy;
        }
        safe_send("you have won\n", winner);
        safe_send("you have lost\n", loser);
    } else{
        safe_send("draw\n", enemy);
        safe_send("draw\n", id);
    }

    remove_client(enemy);
    remove_client(id);
}

void abandon_game(int abandoner){
        int enemy = clients.games[abandoner].second_player;
        clients.games[abandoner].second_player = -1;
        if(waiting_game_id == abandoner) waiting_game_id = -1;
        if(enemy != -1) {
            safe_send("game was abandoned\n", enemy);
            clients.games[enemy].second_player = -1;
            init_game(enemy);
        }
}

void advance_game(char* msg, int player_id){
    if(strcmp(msg, "end")==0){ // end game & remove client
        print("%d abandoned a game\n", player_id);
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
                print("Sending next board state to %d\n", enemy);
                safe_send(buffer, enemy);
            }
        } else {
             print("Sending bad move message\n");
            safe_send("bad move", player_id);
        }
        pthread_mutex_unlock(&clients.mutex_game);
    } else{
        print("Sending bad move message\n");
        safe_send("bad move", player_id);
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
    if ((local_server_socket_fd = socket(AF_LOCAL, SOCKET_PROTOCOL, 0)) == -1)               error("cannot create local socket");
    if ((bind(local_server_socket_fd, (struct sockaddr*) &so_addr, sizeof(so_addr))) == -1)  error("cannot bind local socket");
    // todo int unlink(const char *pathname)
    if ((listen(local_server_socket_fd, SERVER_CLIENTS_LIMIT + 1)) == -1)                    error("cannot listen on local socket");



    // INET socket setup:
    struct hostent * host = gethostbyname("localhost");
    struct in_addr host_address = *(struct in_addr*) host->h_addr;

    struct sockaddr_in sa_inet;
    sa_inet.sin_family = AF_INET;
    sa_inet.sin_addr.s_addr = host_address.s_addr;
    sa_inet.sin_port = htons(inet_port);

    if ((inet_server_socket_fd = socket(AF_INET, SOCKET_PROTOCOL, 0)) == -1)               error("cannot create inet socket");
    if ((bind(inet_server_socket_fd, (struct sockaddr*) &sa_inet, sizeof(sa_inet))) == -1) error("cannot bind inet socket");
    if ((listen(inet_server_socket_fd, SERVER_CLIENTS_LIMIT)) == -1)                       error("cannot listen on inet socket");

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

    LOG("create managersrs\n");
    pthread_t connection_manager_tid;
    pthread_create(&connection_manager_tid, NULL, connection_manager, NULL);

    pthread_t ping_manager_tid;
    pthread_create(&ping_manager_tid, NULL, ping_manager, NULL);

    pthread_t input_manager_tid;
    pthread_create(&input_manager_tid, NULL, game_manager, NULL);

    pthread_join(connection_manager_tid, NULL);
    pthread_join(ping_manager_tid, NULL);
    pthread_join(input_manager_tid, NULL);

}


void* connection_manager(void* arg){
    char msg_buffer[MESSAGE_SIZE];
    forewer{
        poll(sockets, 2, -1);
        int socket = -1;
        for_i_up_to(2) if(sockets[i].revents & POLLIN) socket = sockets[i].fd;
        if(socket == -1) continue;

        int client_fd = accept(socket, NULL, 0);
        LOG("trying to connect (fd=%d)\n", client_fd);
        print("Connection request...\n");
        int client_id = -1;

        pthread_mutex_lock(&clients.mutex_client);
        for_i_up_to(SERVER_CLIENTS_LIMIT) if(clients.fd[i] == -1) client_id = i;
        if(client_id == -1){ // check for free slot for client
            send_message(client_fd, "clients limit reached");
            safe_call(shutdown(client_fd, SHUT_RDWR));
            safe_call(close(client_fd));
            pthread_mutex_unlock(&clients.mutex_client);
        }
        for_i_up_to(SERVER_CLIENTS_LIMIT){ // find free space and addclients
            if(clients.fd[i] == -1){
                receive_message(client_fd, msg_buffer); // waits for name of client
                if(check_existance(msg_buffer)){ // check if name is taken
                    // name already taken
                    safe_call(shutdown(client_fd, SHUT_RDWR));
                    safe_call(close(client_fd));
                    break;
                } else {
                    // name is ok
                    client_id = i;
                    clients.count ++;
                    clients.fd[i] = client_fd; 
                    print("new client accepted | name: >%s<\n", msg_buffer); 
                    strcpy(clients.name[i], msg_buffer);
                    send_message(client_fd, "ok");
                }
                break;
            }
        }
        pthread_mutex_unlock(&clients.mutex_client);
        if(client_id != -1) init_game(client_id);
    }
    return (void*) 0;
}

void* ping_manager(void* arg){
    char buffer[MESSAGE_SIZE];
    forewer{
        pthread_cond_wait(&cond_ping, &mutex_ping);
        for_i_up_to(SERVER_CLIENTS_LIMIT){
            
            if(clients.fd[i] != -1){
                int res = safe_send("ping", i);
                if(res == -1)  {
                    print("Ping failed (id=%d)\n", i);
                    abandon_game(i);
                    remove_client(i); 
                    continue;
                }
                usleep(PING_TIME);
                wrng_msg:
                res = recv(clients.fd[i], buffer, MESSAGE_SIZE, MSG_DONTWAIT);
                if (res <= 0){
                    print("Ping failed (id=%d)\n", i);
                    LOG("ping failed (id=%d | fd=%d) | buffer: %s | res=%d\n", i,clients.fd[i], buffer, res);
                    abandon_game(i);
                    remove_client(i);
                } 
                if(strcmp(buffer, "pong")!=0){
                     safe_send("repeat", i);
                     goto wrng_msg;
                }
            }
        }
        pthread_cond_broadcast(&cond_ping);
    }

    return (void*) 0;
}

void* game_manager(void* arg){
    pthread_mutex_lock(&mutex_ping);
    struct pollfd fds[SERVER_CLIENTS_LIMIT];
    char buffer[MESSAGE_SIZE];
    for_i_up_to(SERVER_CLIENTS_LIMIT){ // update fds
        fds[i].events = POLLIN;
    }
    forewer{
        for_i_up_to(SERVER_CLIENTS_LIMIT){ // update fds
            fds[i].fd = clients.fd[i];
            fds[i].revents = 0;
        }

        poll(fds, SERVER_CLIENTS_LIMIT, 1);
        for_i_up_to(SERVER_CLIENTS_LIMIT){
            if((fds[i].revents & POLLIN) and not(fds[i].revents & POLLHUP) and (clients.games[i].second_player != -1)){ // data to read; game can be advanced
                LOG("revents = %d\n", fds[i].revents);
                LOG("game manager reading data (id=%d | fd=%d)\n", i, fds[i].fd);
                int res = receive_message(fds[i].fd, buffer);
                LOG("msg |%s|\n", buffer);
                if(res == 0){
                    strcpy(buffer, "end");
                } else if (strcmp(buffer, "pong") == 0) continue;
                advance_game(buffer, i);
            }
        }
        pthread_cond_broadcast(&cond_ping);
        pthread_cond_wait(&cond_ping, &mutex_ping);
    }

    return (void*) 0;
}
CC = gcc -Wall


comp:
	$(CC) server.c -lm -pthread -o server
	$(CC) client.c -lm -pthread -o client

all:
	make clear
	make comp
	make run

server:
	./server 1234 server_path

client_local:
	./client name_local local server_path

client_inet:
	./client name_inet inet 127.0.0.1:1234


run:
	./server 1234 server_path
	
test:
	make run

clean:
	make clear

clear:
	rm -f server
	rm -f client

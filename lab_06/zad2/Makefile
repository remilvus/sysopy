CC = gcc -Wall


comp:
	$(CC) server.c -lrt -lm -o server
	$(CC) client.c -lrt -lm -o client

crun:
	make comp
	make run

all:
	make clear
	make comp

run:
	./server

client:
	./client

server:
	./server

clean:
	make clear

clear:
	rm -f server
	rm -f client

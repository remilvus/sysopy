CC = gcc -Wall


comp:
	$(CC) server.c -lm -o server
	$(CC) client.c -lm -o client

crun:
	make comp
	make run

all:
	make clear
	make comp

run:
	./server

clean:
	make clear

clear:
	rm -f server
	rm -f client

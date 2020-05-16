CC = gcc -Wall


comp:
	$(CC) server.c -lm -pthread -o server
	$(CC) client.c -lm -pthread -o client

all:
	make clear
	make comp
	make run

server:
	./server

client:
	./client

run:
	./server
	
test:
	make run

clean:
	make clear

clear:
	rm -f main
	rm -f result

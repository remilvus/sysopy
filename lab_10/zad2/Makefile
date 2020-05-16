CC = gcc -Wall


comp:
	$(CC) client.c -lm -pthread -o client
	$(CC) server.c -lm -pthread -o server
	

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

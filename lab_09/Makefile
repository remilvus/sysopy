CC = gcc -Wall


comp:
	$(CC) main.c -lm -lpthread -o main

all:
	make clear
	make comp
	./main 3 8

run:
	./main 3 8
	
test:
	make run

clean:
	make clear

clear:
	rm -f main
	rm -f result

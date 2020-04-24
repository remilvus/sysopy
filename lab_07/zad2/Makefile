CC = gcc -Wall


comp:
	$(CC) main.c -lrt -pthread -o main
	$(CC) first.c -pthread -lrt -lm -o first
	$(CC) second.c -pthread -lrt -lm -o second
	$(CC) third.c -pthread -lrt -lm -o third

all:
	make clear
	make comp
	make run

run:
	./main

test:
	./main

clean:
	make clear

clear:
	rm -f main
	rm -f first
	rm -f second
	rm -f third

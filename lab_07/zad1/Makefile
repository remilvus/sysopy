CC = gcc -Wall


comp:
	$(CC) main.c -lm -o main
	$(CC) first.c -lm -o first
	$(CC) second.c -lm -o second
	$(CC) third.c -lm -o third

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

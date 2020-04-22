CC = gcc -Wall


comp:
	$(CC) main.c -lm -o main

crun:
	make comp
	make run

all:
	make clear
	make comp

ignore:
	./main ignore

handler:
	./main handler

mask:
	./main mask

pending:
	./main pending



clean:
	make clear

clear:
	rm -f main
	rm -f *.out

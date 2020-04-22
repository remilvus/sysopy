CC = gcc -Wall


comp:
	$(CC) main.c -lm -o main

crun:
	make comp
	make run

all:
	make clear
	make comp

run:
	./main 

clean:
	make clear

clear:
	rm -f main
	rm -f *.out

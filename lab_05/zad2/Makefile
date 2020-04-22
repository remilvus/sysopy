CC = gcc -Wall


comp:
	$(CC) main.c -lm -o main

crun:
	make comp
	make run

make all:
	make clear
	make comp

run:
	./main tekst.txt

clean:
	make clear

clear:
	rm -f main
	rm -f *.out

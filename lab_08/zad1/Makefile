CC = gcc -Wall


comp:
	$(CC) main.c -lm -lpthread -o main

all:
	make clear
	make comp
	./main 4 sign baboon.pgm result_sign
	./main 4 block baboon.pgm result_block
	./main 4 interleaved baboon.pgm result_inter


run:
	./main 4 block baboon.pgm result

test:
	make run

clean:
	make clear

clear:
	rm -f main
	rm -f result

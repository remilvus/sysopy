CC = gcc -Wall


comp:
	$(CC) Reader.c -lm -o reader
	$(CC) Writer.c -lm -o writer
	$(CC) main.c -lm -o main

crun:
	make comp
	make run

make all:
	make clear
	make comp

test:
	make run

fifo:
	mkfifo fifo

run:
	./main

clean:
	make clear

clear:
	rm -f main
	rm -f *.out
	rm -f fifo
	rm -f reader
	rm -f writer
	rm -f out.txt

CC = gcc -Wall


comp:
	$(CC) main.c -o main

test_time:
	./main .. main.c -atime -10 -mtime -10

test_traverse:
	./main . -noname

test_depth:
	./main . depth2.txt -maxdepth 2

clear:
	rm -f *.txt
	rm -f main
	rm -f *.out

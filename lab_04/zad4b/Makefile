CC = gcc -Wall


comp:
	$(CC) sender.c -lm -o sender
	$(CC) catcher.c -lm -o catcher

all:
	make clear
	make comp

catch:
	./catcher kill

catchque:
	./catcher sigqueue

catchrt:
	./catcher sigrt

clean:
	make clear

clear:
	rm -f sender
	rm -f catcher
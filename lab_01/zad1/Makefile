CC = gcc -Wall

static:
	make clean
	$(CC) -c mylib.c
	ar crs libmylib.a mylib.o

shared:
	make clean
	$(CC) -fPIC -c mylib.c
	$(CC) -shared -fPIC -o libmylib.so mylib.o

clean:
	rm -f *.o
	rm -f *.a
	rm -f *.so
	rm -f main

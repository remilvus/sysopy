CC = gcc -Wall


static:
	make clean
	$(CC) -c mylib.c
	ar crs libmylib.a mylib.o
	$(CC) main.c -o main -L. -lmylib

shared:
	make clean
	$(CC) -fPIC -c mylib.c
	$(CC) -shared -fPIC -o libmylib.so mylib.o
	$(CC) main.c -o main -L. -lmylib -Wl,-rpath=`pwd`

dynamic:
	make clean
	$(CC) -fPIC -c mylib.c -o mylib.o 
	$(CC) -Wl,-rpath=. -fPIC -shared -o libmylib.so mylib.o 
	$(CC) -fPIC -L. -o main main_dynamic.c -ldl -D DYNAMIC 

clean:
	rm -f main
	rm -f main_dynamic
	rm -f tmp.txt
	rm -f *.so
	rm -f *.o
	rm -f *.a


test:
	./main create_table 10 compare_pairs ../big.txt:../big2.txt compare_pairs a.txt:b.txt compare_pairs ../big.txt:../big2.txt



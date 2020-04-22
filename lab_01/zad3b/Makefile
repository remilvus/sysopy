CC = gcc -Wall


static:
	make clean
	$(CC) -O0 -c mylib.c
	ar crs libmylib.a mylib.o
	$(CC) -O0 main.c -o main -L. -lmylib

static_s:
	make clean
	$(CC) -Os -c mylib.c
	ar crs libmylib.a mylib.o
	$(CC) -Os main.c -o main -L. -lmylib

static_2:
	make clean
	$(CC) -O2 -c mylib.c
	ar crs libmylib.a mylib.o
	$(CC) -O2 main.c -o main -L. -lmylib

shared:
	make clean
	$(CC) -O0 -fPIC -c mylib.c
	$(CC) -O0 -shared -fPIC -o libmylib.so mylib.o
	$(CC) -O0 main.c -o main -L. -lmylib -Wl,-rpath=`pwd`

shared_s:
	make clean
	$(CC) -Os -fPIC -c mylib.c
	$(CC) -Os -shared -fPIC -o libmylib.so mylib.o
	$(CC) -Os main.c -o main -L. -lmylib -Wl,-rpath=`pwd`

shared_2:
	make clean
	$(CC) -O2 -fPIC -c mylib.c
	$(CC) -O2 -shared -fPIC -o libmylib.so mylib.o
	$(CC) -O2 main.c -o main -L. -lmylib -Wl,-rpath=`pwd`

dynamic:
	make clean
	$(CC) -O0 -fPIC -c mylib.c -o mylib.o 
	$(CC) -O0 -Wl,-rpath=. -fPIC -shared -o libmylib.so mylib.o 
	$(CC) -O0 -fPIC -L. -o main main_dynamic.c -ldl -D DYNAMIC

dynamic_s:
	make clean
	$(CC) -Os -fPIC -c mylib.c -o mylib.o 
	$(CC) -Os -Wl,-rpath=. -fPIC -shared -o libmylib.so mylib.o 
	$(CC) -Os -fPIC -L. -o main main_dynamic.c -ldl -D DYNAMIC

dynamic_2:
	make clean
	$(CC) -O2 -fPIC -c mylib.c -o mylib.o 
	$(CC) -O2 -Wl,-rpath=. -fPIC -shared -o libmylib.so mylib.o 
	$(CC) -O2 -fPIC -L. -o main main_dynamic.c -ldl -D DYNAMIC

clean:
	rm -f main
	rm -f main_dynamic
	rm -f tmp.txt
	rm -f *.so
	rm -f *.o
	rm -f *.a


test:
	./main create_table 10 compare_pairs ../big.txt:../big2.txt compare_pairs a.txt:b.txt compare_pairs ../big.txt:../big2.txt


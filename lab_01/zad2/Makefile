CC = gcc -Wall

all:
	make clean
	make shared

shared:
	make clean
	$(CC) -fPIC -c mylib.c
	$(CC) -shared -fPIC -o libmylib.so mylib.o
	$(CC) main.c -o main -L. -lmylib -Wl,-rpath=`pwd`

test:
	./main create_table 100 compare_pairs ../big.txt:../big2.txt compare_pairs a.txt:b.txt remove_block 0 remove_block 1 compare_pairs ../big.txt:../big2.txt compare_pairs a.txt:b.txt remove_block 0 remove_block 1 compare_pairs ../big.txt:../big2.txt compare_pairs a.txt:b.txt remove_block 0 remove_block 1 compare_pairs a.txt:a.txt a.txt:a_similar.txt a.txt:a_less_similar.txt compare_pairs a.txt:a.txt a.txt:a_similar.txt a.txt:a_less_similar.txt a_similar.txt:a_less_similar.txt b.txt:b.txt b.txt:b_similar.txt b_similar.txt:b_less_similar.txt b.txt:b_less_similar.txt compare_pairs a.txt:a.txt a.txt:a_similar.txt a.txt:a_less_similar.txt a_similar.txt:a_less_similar.txt b.txt:b.txt b.txt:b_similar.txt b_similar.txt:b_less_similar.txt b.txt:b_less_similar.txt a.txt:b.txt a.txt:b_similar.txt a_similar.txt:b_less_similar.txt a.txt:b_less_similar.txt a.txt:a.txt a.txt:a_similar.txt a.txt:a_less_similar.txt a_similar.txt:a_less_similar.txt b.txt:b.txt b.txt:b_similar.txt b_similar.txt:b_less_similar.txt b.txt:b_less_similar.txt a.txt:b.txt a.txt:b_similar.txt a_similar.txt:b_less_similar.txt a.txt:b_less_similar.txt

clean:
	rm -f *.o
	rm -f *.a
	rm -f *.so
	rm -f main
	rm -f tmp.txt

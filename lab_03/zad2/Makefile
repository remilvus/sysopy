CC = gcc -Wall


comp:
	$(CC) main.c -lm -o macierz
	$(CC) generator.c -o generator

crun:
	make comp
	make run

make all:
	make clear
	make comp
	make gen

everything:
	make check
	make clear
	make comp
	make gen
	make run

example:
	./generator mtest
	./macierz test 2 10

check:
	make clear
	make comp
	./generator mtest
	./macierz test 2 10
	sleep 1
	./generator assert

gen:
	./generator inl_1.txt inr_1.txt inl_2.txt inr_2.txt 

run:
	./macierz inputs 5 10

test:
	make run

clean:
	make clear

clear:
	rm -f *.txt
	rm -f macierz
	rm -f generator
	rm -f *.out

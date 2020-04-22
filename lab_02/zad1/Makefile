CC = gcc -Wall
file = random.txt
filec = copy.txt
size = 10000
len = 100

define run_test
	./main generate $(file) $(1) $(2) \
	copy $(file) $(filec) $(1) $(2) syscopy $(file) $(filec) $(1) $(2) \
	sort $(file) $(1) $(2) copy $(filec) $(file) $(1) $(2) libsort $(file) $(1) $(2)
endef

comp:
	$(CC) main.c util.c quicksort.c -o main

test:
	$(call run_test,20000,1)
	$(call run_test,40000,1)
	$(call run_test,90000,2)
	$(call run_test,120000,2)
	$(call run_test,90000,4)
	$(call run_test,120000,4)
	$(call run_test,150000,512)
	$(call run_test,250000,512)
	$(call run_test,150000,1024)
	$(call run_test,250000,1024)
	$(call run_test,150000,4096)
	$(call run_test,250000,4096)
	$(call run_test,150000,8192)
	$(call run_test,250000,8192)

c:
	./main copy $(file) $(filec) $(size) $(len) syscopy $(file) $(filec) $(size) $(len)

r:
	./main copy $(filec) $(file) $(size) $(len)

g:
	./main generate $(file) $(size) $(len)

s:
	./main sort $(file) $(size) $(len) libsort $(file) $(size) $(len)

clear:
	rm -f *.out
	rm -f main

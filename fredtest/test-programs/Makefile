all: pthread-test test-list syscall-tester pthread-cond-var

clean:
	rm -f pthread-test test-list syscall-tester

pthread-test: pthread-test.c
	gcc -o pthread-test pthread-test.c -g -O0 -lpthread

pthread-cond-var: pthread-cond-var.c
	gcc -o pthread-cond-var pthread-cond-var.c -g -O0 -lpthread

test-list: test-list.c
	gcc -o test-list test-list.c -g -O0

syscall-tester: syscall-tester.c
	gcc -o syscall-tester syscall-tester.c -g -O0 -DLINUX -DGLIBC

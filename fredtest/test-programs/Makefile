all: pthread-test test-list

clean:
	rm -f pthread-test test-list

pthread-test: pthread-test.c
	gcc -o pthread-test pthread-test.c -g -O0 -lpthread

test-list: test-list.c
	gcc -o test-list test-list.c -g -O0

CC = gcc

sh: sh.c
	$(CC) -o sh sh.c

test: sh t.sh
	./sh < t.sh

clean:
	rm -f sh
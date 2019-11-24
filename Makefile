CC = gcc

shell : shell.c filesys.c
	$(CC) -o shell shell.c filesys.c -std=c99

run:
	./shell

clean:
	rm -f shell
CC = gcc

shell : shell.c filesys.c
	$(CC) -o shell shell.c filesys.c -std=c99

run:
	./shell

hex: virtualdiskC3_C1
	hexdump -C virtualdiskC3_C1

clean:
	rm -f shell
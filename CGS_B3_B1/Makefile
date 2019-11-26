CC = gcc

shell : shell.c filesys.c
	$(CC) -o shell shell.c filesys.c -std=c99

run:
	./shell

hex: virtualdiskB3_B1
	hexdump -C virtualdiskB3_B1

clean:
	rm -f shell
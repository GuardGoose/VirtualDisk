CC = gcc

shell : shell.c filesys.c
	$(CC) -o shell shell.c filesys.c

run:
	./shell

hex: virtualdiskD3_D1
	hexdump -C virtualdiskD3_D1

clean:
	rm -f shell
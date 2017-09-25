
all: shell
shell: mytoc.o shell.o
	cc -o shell shell.o mytoc.o

clean:
	rm -f *.o *.exe

mytoc:  mytoc.o
	cc -o mytoc.c  mytoc.o

run: all
	./shell

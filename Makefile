CC=gcc
CFLAGS=-I. -ansi -pedantic -Wall

main: project1.o
	$(CC) $(CCFLAGS) -g -o main project1.o
project1.o: project1.c
	$(CC) $(CCFLAGS) -g -c project1.c
clean:
	rm -f *.o main  

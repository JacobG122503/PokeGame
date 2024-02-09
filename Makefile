Program: Program.o heap.o
	gcc Program.o heap.o -o Program

clean:
	rm -f Program *~ core *.o

brun: Program.o
	gcc Program.c -o Program
	./Program

Program.o: Program.c 
	gcc -Wall -Werror -g Program.c -c 

heap.o: heap.c heap.h
	gcc -Wall -Werror -g heap.c -c

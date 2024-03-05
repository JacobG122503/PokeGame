Program: Program.o heap.o
	gcc Program.o heap.o -o Program -lncurses

clean:
	rm -f Program *~ core *.o seeds.txt

brun: Program.o heap.o
	gcc Program.o heap.o -o Program -lncurses
	./Program

Program.o: Program.c 
	gcc -Wall -Werror -g Program.c -c 

heap.o: heap.c heap.h
	gcc -Wall -Werror -g heap.c -c

Program: Program.o heap.o
	g++ Program.o heap.o -o Program -lncurses

clean:
	rm -f Program *~ core *.o seeds.txt

brun: Program.o heap.o
	g++ Program.o heap.o -o Program -lncurses
	./Program

Program.o: Program.cpp 
	g++ -Wall -Werror -g Program.cpp -c 

heap.o: heap.cpp heap.h
	g++ -Wall -Werror -g heap.cpp -c

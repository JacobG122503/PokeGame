Program: Program.c
	gcc -Wall -Werror -g Program.c -o Program

clean:
	rm -f Program *~ core 

brun: Program.c
	gcc Program.c -o Program
	./Program

#a make file used to compile our progaram
all:mysh.c
	gcc -g -Wall -o mysh mysh.c

clean:
	$(RM) mysh

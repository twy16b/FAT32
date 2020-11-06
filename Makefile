PROG = project3
CC = gcc
OBJS = project3.o commands.o helpers.o
CFLAGS =

$(PROG): $(OBJS)
	gcc -o $(PROG) $(OBJS)
	rm $(OBJS)
project3.o:
	gcc -c $(CFLAGS) project3.c
commands.o:
	gcc -c $(CFLAGS) commands.c
helpers.o:
	gcc -c $(CFLAGS) helpers.c
clean:
	rm $(OBJS) project3
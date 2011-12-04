CC = gcc
LIBS = /home/courses/cse533/Stevens/unpv13e/libunp.a -lpthread
FLAGS = -g -O2 -w
CFLAGS = ${FLAGS} -I/home/courses/cse533/Stevens/unpv13e/lib

all: tour

tour: tour.o
	${CC} ${CFLAGS} -o tour tour.o ${LIBS}
tour.o:
	${CC} ${CFLAGS} -c tour.c
clean:
	rm -f tour.o tour

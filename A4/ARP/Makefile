
CC = gcc
LIBS = /home/courses/cse533/Stevens/unpv13e/libunp.a -lpthread
FLAGS = -g -O2 
CFLAGS = ${FLAGS} -I/home/courses/cse533/Stevens/unpv13e/lib

all: arp 
	

###client
arp: arp.o
	${CC} ${CFLAGS} -o arp arp.o ${LIBS}
arp.o:
	${CC} ${CFLAGS} -c arp.c


clean:
	rm -f  arp.o arp.bak

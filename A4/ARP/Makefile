
CC = gcc
LIBS = /home/courses/cse533/Stevens/unpv13e/libunp.a -lpthread
FLAGS = -g -O2 
CFLAGS = ${FLAGS} -I/home/courses/cse533/Stevens/unpv13e/lib

all: arp api
	
api: api.o
	${CC} ${CFLAGS} -o api api.o ${LIBS}
api.o:
	${CC} ${CFLAGS} -c api.c	
	
get_hw_addrs.o: get_hw_addrs.c
	${CC} ${CFLAGS} -c get_hw_addrs.c
###client
arp: arp.o get_hw_addrs.o
	${CC} ${CFLAGS} -o arp arp.o get_hw_addrs.o ${LIBS}
arp.o:
	${CC} ${CFLAGS} -c arp.c


clean:
	rm -f  arp arp.o arp.bak api api.o

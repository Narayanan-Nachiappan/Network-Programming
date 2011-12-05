
CC = gcc
LIBS = /home/courses/cse533/Stevens/unpv13e/libunp.a -lpthread
FLAGS = -g -O2 
CFLAGS = ${FLAGS} -I/home/courses/cse533/Stevens/unpv13e/lib

all: MulticastSend MulticastReceive 
	


MulticastSend: MulticastSend.o
	${CC} ${CFLAGS} -o MulticastSend MulticastSend.o ${LIBS}
MulticastSend.o:
	${CC} ${CFLAGS} -c MulticastSend.c
MulticastReceive: MulticastReceive.o
	${CC} ${CFLAGS} -o MulticastReceive MulticastReceive.o ${LIBS}
MulticastReceive.o:
	${CC} ${CFLAGS} -c MulticastReceive.c


CC = gcc
LIBS = /home/courses/cse533/Stevens/unpv13e/libunp.a -lpthread
FLAGS = -g -O2 
CFLAGS = ${FLAGS} -I/home/courses/cse533/Stevens/unpv13e/lib

all: arp 
	

###client
api: api.o
	${CC} ${CFLAGS} -o api api.o ${LIBS}
api.o:
	${CC} ${CFLAGS} -c api.c

multiserver: multiserver.o
	${CC} ${CFLAGS} -o multiserver multiserver.o ${LIBS}
multiserver.o:
	${CC} ${CFLAGS} -c multiserver.c
multiclient: multiclient.o
	${CC} ${CFLAGS} -o multiclient multiclient.o ${LIBS}
multiclient.o:
	${CC} ${CFLAGS} -c multiclient.c

clean:
	rm -f  arp.o arp.bak

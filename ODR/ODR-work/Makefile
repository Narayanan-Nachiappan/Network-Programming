
CC = gcc
LIBS = /home/courses/cse533/Stevens/unpv13e/libunp.a -lpthread
FLAGS = -g -O2 -w
CFLAGS = ${FLAGS} -I/home/courses/cse533/Stevens/unpv13e/lib

all:  client_group15 server_group15 odr_group15



###client_group15
client_group15: client_group15.o
	${CC} ${CFLAGS} -o client_group15 client_group15.o ${LIBS}
client_group15.o:
	${CC} ${CFLAGS} -c client_group15.c

###server_group15
server_group15: server_group15.o
	${CC} ${CFLAGS} -o server_group15 server_group15.o ${LIBS}
server_group15.o:
	${CC} ${CFLAGS} -c server_group15.c

###odr_group15
route.o: route.c
	${CC} ${CFLAGS} -c route.c
demux.o: demux.c
	${CC} ${CFLAGS} -c demux.c	
odr_group15.o: odr_group15.c
	${CC} ${CFLAGS} -c odr_group15.c
odr_group15: odr_group15.o demux.o route.o get_hw_addrs.o
	${CC} ${FLAGS} -o odr_group15 odr_group15.o demux.o route.o get_hw_addrs.o ${LIBS}


clean:
	rm -f  client_group15 client_group15.o client_group15.bak server_group15 server_group15.o server_group15.bak odr_group15 odr_group15.o demux.o route.o


CC = gcc
LIBS = /home/courses/cse533/Stevens/unpv13e/libunp.a -lpthread
FLAGS = -g -O2 
CFLAGS = ${FLAGS} -I/home/courses/cse533/Stevens/unpv13e/lib

all: domainclient domainserver client server

#domainclient
domainclient: domainclient.o
	${CC} ${CFLAGS} -o domainclient domainclient.o ${LIBS}
domainclient.o:
	${CC} ${CFLAGS} -c domainclient.c


####domain server
domainserver: domainserver.o
	${CC} ${CFLAGS} -o domainserver domainserver.o ${LIBS}
domainserver.o:
	${CC} ${CFLAGS} -c -o domainserver.o domainserver.c


###client
client: client.o
	${CC} ${CFLAGS} -o client client.o ${LIBS}
client.o:
	${CC} ${CFLAGS} -c client.c
###server
server: server.o
	${CC} ${CFLAGS} -o server server.o ${LIBS}
server.o:
	${CC} ${CFLAGS} -c server.c

###miniodr
route.o: route.c
	${CC} ${CFLAGS} -c route.c
demux.o: demux.c
	${CC} ${CFLAGS} -c demux.c	
odr.o: odr.c
	${CC} ${CFLAGS} -c odr.c
odr: odr.o demux.o route.o get_hw_addrs.o
	${CC} ${FLAGS} -o odr odr.o demux.o route.o get_hw_addrs.o ${LIBS}


clean:
	rm -f domainclient.o domainserver.o domainclient domainserver *.bak client.o client.bak server.o server.bak odr odr.o demux.o route.o

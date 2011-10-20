
CC = gcc

LIBS =  -lsocket\
	/home/courses/cse533/Stevens/unpv13e_solaris2.10/libunp.a

FLAGS =  -g -O2
CFLAGS = ${FLAGS} -I/home/courses/cse533/Stevens/unpv13e_solaris2.10/lib

all: server client

mask.o: mask.c
	${CC} ${CFLAGS} -c mask.c

client: udpcli01.o
	${CC} ${FLAGS} -o client udpcli01.o ${LIBS}
udpcli01.o: udpcli01.c
	${CC} ${CFLAGS} -c udpcli01.c
	
get_ifi_info_plus.o: get_ifi_info_plus.c
	${CC} ${CFLAGS} -c get_ifi_info_plus.c

server: udpserver.o get_ifi_info_plus.o mask.o
	${CC} ${FLAGS} -o server udpserver.o get_ifi_info_plus.o mask.o ${LIBS}
udpserver.o: udpserver.c
	${CC} ${CFLAGS} -c udpserver.c

clean:
	rm server udpserver.o get_ifi_info_plus.o mask.o client udpcli01.o


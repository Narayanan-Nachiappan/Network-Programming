
CC = gcc

LIBS =  -lsocket\
	/home/courses/cse533/Stevens/unpv13e_solaris2.10/libunp.a

FLAGS =  -g -O2
CFLAGS = ${FLAGS} -I/home/courses/cse533/Stevens/unpv13e_solaris2.10/lib

all: server client

mask.o: mask.c
	${CC} ${CFLAGS} -c mask.c
client: client.o get_ifi_info_plus.o mask.o
	${CC} ${FLAGS} -o client client.o get_ifi_info_plus.o ${LIBS}
dg_send_recv.o: dg_send_recv.c
	${CC} ${CFLAGS} -c dg_send_recv.c
get_ifi_info_plus.o: get_ifi_info_plus.c
	${CC} ${CFLAGS} -c get_ifi_info_plus.c
server: server.o get_ifi_info_plus.o mask.o dg_send_recv.o
	${CC} ${FLAGS} -o server server.o get_ifi_info_plus.o mask.o dg_send_recv.o ${LIBS}

clean:
	rm server server.o get_ifi_info_plus.o mask.o client client.o dg_send_recv.o


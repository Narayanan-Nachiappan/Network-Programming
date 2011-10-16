include ../Make.defines

PROGS =	udpcli01

all:	${PROGS}

udpcli01:	udpcli01.o dg_cli.o dg_send_recv.o
		${CC} ${CFLAGS} -o $@ udpcli01.o dg_cli.o dg_send_recv.o ${LIBS}


udpserv01:	udpserv01.o
		${CC} ${CFLAGS} -o $@ udpserv01.o ${LIBS}


clean:
		rm -f ${PROGS} ${CLEANFILES}

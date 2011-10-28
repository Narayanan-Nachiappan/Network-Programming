include ../Make.defines

PROGS =	udpcli01 updserv01

all:	${PROGS}

udpcli01:	udpcli01.o  
		${CC} ${CFLAGS} -o $@ udpcli01.o   ${LIBS}
udpserv01:	udpserv01.o dg_send_recv.o
		${CC} ${CFLAGS} -o $@ dg_send_recv.o udpserv01.o ${LIBS}
udpcli:		udpcli.o  
		${CC} ${CFLAGS} -o $@ udpcli.o   ${LIBS}
udpclitry:	udpclitry.o  dg_send_recv_cli.o
		${CC} ${CFLAGS} -o $@ udpclitry.o dg_send_recv_cli.o   ${LIBS}


clean:
		rm -f ${PROGS} ${CLEANFILES}


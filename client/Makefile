include ../Make.defines

PROGS =	a2_cli a2_srv

all:	${PROGS}

a2_cli:	a2_cli.o
		${CC} ${CFLAGS} -o $@ a2_cli.o ${LIBS}

a2_srv: a2_srv.o
		${CC} ${CFLAGS} -o $@ a2_srv.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}

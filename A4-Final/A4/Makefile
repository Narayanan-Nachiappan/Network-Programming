CC = gcc
LIBS = /home/courses/cse533/Stevens/unpv13e/libunp.a -lpthread
FLAGS = -g -O2 -w
CFLAGS = ${FLAGS} -I/home/courses/cse533/Stevens/unpv13e/lib

all: tour arp

tour: tour.o api.o get_hw_addrs.o ping.o
	${CC} ${CFLAGS} -o tour tour.o api.o get_hw_addrs.o ping.o ${LIBS}
arp: arp.o get_hw_addrs.o
	${CC} ${CFLAGS} -o arp arp.o get_hw_addrs.o ${LIBS}
ping.o:
	${CC} ${CFLAGS} -c ping.c
arp.o:
	${CC} ${CFLAGS} -c arp.c
get_hw_addrs.o:
	${CC} ${CFLAGS} -c get_hw_addrs.c
tour.o:
	${CC} ${CFLAGS} -c tour.c
api.o:
	${CC} ${CFLAGS} -c api.c

clean:
	rm -f tour.o tour api.o arp arp.o get_hw_addrs.o ping.o

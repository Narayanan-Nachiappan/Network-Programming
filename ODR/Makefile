
CC = gcc

LIBS =  /home/courses/cse533/Stevens/unpv13e/libunp.a

FLAGS =  -g -O2
CFLAGS = ${FLAGS} -I/home/courses/cse533/Stevens/unpv13e/lib

all: odr miniodr

odr.o: odr.c
	${CC} ${CFLAGS} -c odr.c
odr: odr.o get_hw_addrs.o
	${CC} ${FLAGS} -o odr odr.o get_hw_addrs.o ${LIBS}
miniodr.o: miniodr.c
	${CC} ${CFLAGS} -c miniodr.c
miniodr: miniodr.o get_hw_addrs.o
	${CC} ${FLAGS} -o miniodr miniodr.o get_hw_addrs.o ${LIBS}

clean:
	rm odr odr.o miniodr miniodr.o get_hw_addrs.o


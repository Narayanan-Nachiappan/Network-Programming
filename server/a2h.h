#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>


#define MAXSOCKS 20

/*
#define MAXLINE 1024

#ifndef max
	#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif*/
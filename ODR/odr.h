#include <errno.h>
#include "hw_addrs.h"
#include <linux/if_ether.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netpacket/packet.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <time.h>

#define ODR_SUNPATH "tmp/group15odr"
#define ODR_PROTOCOL 32325
#define TIME_SERV_SUNPATH "cse533_unixdg_path"
#define MAX_INTERFACES 10
#define MAX_ROUTES 10
#define MAXLINE 1024

#ifndef max
	#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif


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

#define ODR_SUNPATH "tmp/group15odr\0"
#define ODR_PROTOCOL 32325
#define TIME_SERV_SUNPATH "cse533_unixdg_path\0"
#define	SUNPATH_ROOT	"group15\0"
#define TIME_PORT 44444
#define MAX_INTERFACES 10
#define MAX_ROUTES 10
#define MAXLINE 1024

#ifndef max
	#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif

struct demux
{
	int 			port;
	char			sun_path[30];
	time_t			timestamp;
	struct demux	*next;
};

struct route
{
	char			nexthop_ip[16];
	char 			nexthop_haddr[6];
	int				index; 
	int 			hops; 
	time_t 			timestamp;
};

struct payload
{
	int 	srcport;
	int 	destport;
	int		msgsz;
	char	*message;
};

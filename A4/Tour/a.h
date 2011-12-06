#include "unp.h"
#include <string.h>

#define RTPROTO 15153
#define ID 15153
#define RTPORT 53535
#define MULTIADDR "224.0.0.0"
#define MULTIPORT 555555
#define SIZE 1024 /* Buffer size */

struct ip {
   unsigned int   ip_hl:4; /* both fields are 4 bits */
   unsigned int   ip_v:4;
   uint8_t        ip_tos;
   uint16_t       ip_len;
   uint16_t       ip_id;
   uint16_t       ip_off;
   uint8_t        ip_ttl;
   uint8_t        ip_p;
   uint16_t       ip_sum;
   struct in_addr ip_src;
   struct in_addr ip_dst;
};

struct IpAddress{
	char ipAddr[INET_ADDRSTRLEN];
};

struct Tour{
	int numNodes;
	int *nodes;
	struct IpAddress *addrs;
	int index;
	struct IpAddress mtAddr;
	int mtPort;
};

int isNumber(char str[]){
	int withDecimal=0,isNegative=0 ,i=0;
	int len = strlen(str);

	for (i=0; i<len; i++){
		if (!isdigit(str[i])){
			return 0;
		}
	}
	return 1;
}
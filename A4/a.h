#include "arp.h"
#include "unp.h"
#include <string.h>

#define RTPROTO 199
#define ID 1515
#define RTPORT 53534
#define MULTIADDR "224.0.0.0"
#define MULTIPORT 55555

struct IpAddress{
	char ipAddr[INET_ADDRSTRLEN];
};

struct Tour{
	int numNodes;
	int nodes[20];
	struct IpAddress addrs[20];
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
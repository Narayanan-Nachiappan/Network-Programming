#include "unp.h"
#include <string.h>

#define RTPROTO 7
#define ID 1515
#define RTPORT 53534
#define MULTIADDR "224.0.0.0"
#define MULTIPORT 555555
#define SIZE 1024 /* Buffer size */

struct IpAddress{
	char ipAddr[15];
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
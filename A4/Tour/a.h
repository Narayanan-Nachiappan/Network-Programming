#include "unp.h"
#include <string.h>

#define RTPROTO = 15153;

struct IpAddress{
	char ipAddr[INET_ADDRSTRLEN];
};

struct Tour{
	int numNodes;
	int *nodes;
	struct IpAddress *addrs;
	int index;
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
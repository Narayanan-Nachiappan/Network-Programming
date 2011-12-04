#include "unp.h"
#include "a.h"
#include <string.h>

void initTour(struct Tour*, int, char**);

int main(int argc, char **argv){
	err_msg("CSE 533 : Network Programming");
	err_msg("Amelia Ellison - 107838108");
	err_msg("Narayanan Nachiappan - 107996031");
	err_msg("Youngbum Kim - 107387376");
	err_msg("Assignment #4");
	err_msg("----------------------------------------");
	err_msg("tour.c");
	err_msg("----------------------------------------");

	struct Tour tour;
	initTour(&tour, argc, argv); /* Create list of visiting nodes and their addresses. */
	printVisitingNode(&tour); /* Print list of visitinrg nodes and their addresses */

	int rtSockfd;
	if(rtSockfd = socket(AF_INET, SOCK_RAW, RTTPROTO) < 0){
		err_msg("rt sock: %d %s\n", errno, strerror(errno));
	}
	
	const int on = 1;
	if(setsockopt(rtSockfd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0){
		err_msg("rt sock: %d %s\n", errno, strerror(errno));
	}

}

void initTour(struct Tour *tour, int size, char **argv){
	char hostName[10];
	char ipAddr[INET_ADDRSTRLEN];
	int *nodes;
	struct ipAddress *ipAddrs;
	char nodeName[5];
	
	struct hostent *hptr;
	struct in_addr **pptr;
	int i;

	gethostname(hostName, sizeof(hostName)); /* Get Host name for the node */
	hptr = gethostbyname(hostName);
	pptr = (struct in_addr**) hptr->h_addr_list;
	Inet_ntop(hptr->h_addrtype, *pptr, ipAddr, sizeof(ipAddr));
	ipAddrs = (struct ipAddress *)malloc( size * sizeof(struct IpAddress));
	struct IpAddress* addr;
	addr = (struct IpAddress *)malloc(sizeof(struct IpAddress));
	strcpy(addr->ipAddr, ipAddr);
	memcpy((void*)ipAddrs, (void*)addr, sizeof(struct IpAddress));


	err_msg("Host Name: %s (%s)", hostName, ipAddr);
	err_msg("----------------------------------------");
	err_msg("Number of visiting nodes: %d", size-1);
	nodes = (int *)malloc( size * sizeof(int));

	strcpy(nodeName, strtok(hostName, "vm"));
	nodes[0] = atoi(nodeName);

	for(i=1;i < size;i++){
		if(strtok(argv[i], "vm") == NULL) err_quit("Invalid argument : %s", argv[i]);
		else strcpy(nodeName, strtok(argv[i], "vm"));
		if(!isNumber(nodeName))	err_quit("Invalid argument : %s", argv[i]);
		int j = atoi(nodeName);
		if(j > 10) err_quit("Invalid argument: %s", argv[i]);
		nodes[i] = j;

		hptr = gethostbyname(argv[i]);

		hptr = gethostbyname(argv[i]);
		pptr = (struct in_addr**) hptr->h_addr_list;
		Inet_ntop(hptr->h_addrtype, *pptr, ipAddr, sizeof(ipAddr));
		strcpy(addr->ipAddr, ipAddr);
		memcpy((void*)ipAddrs + ( i * sizeof(struct IpAddress) ), (void*)addr, sizeof(struct IpAddress));
	}
	
	tour->numNodes = size;
	tour->nodes = nodes;
	tour->addrs = ipAddrs;
	tour->index = 1;
}

void printVisitingNode(struct Tour *tour){
	err_msg("List of visiting nodes");
	int i;
	for(i = 0; i< tour->numNodes; i++){
		if(i == 0) err_msg("Source: vm%d (%s)", tour->nodes[i], tour->addrs[i].ipAddr);
		else err_msg("vm%d (%s)", tour->nodes[i], tour->addrs[i].ipAddr);
	}
}

#include "unp.h"
#include "a.h"
#include <string.h>

void initTour(struct Tour*, int, char**);
void printVisitingNode(struct Tour*);
void joinMTGroup(int, char*, unsigned short);
void fillHdr(char*, int, struct sockaddr_in*, struct sockaddr_in*);

int main(int argc, char **argv){
	err_msg("CSE 533 : Network Programming");
	err_msg("Amelia Ellison - 107838108");
	err_msg("Narayanan Nachiappan - 107996031");
	err_msg("Youngbum Kim - 107387376");
	err_msg("Assignment #4");
	err_msg("----------------------------------------");
	err_msg("tour.c");
	err_msg("----------------------------------------");

	int rtSockfd;				/* rt socket */
	int mtSockfd_send;			/* mt socket (Sending) */
	int mtSockfd_recv;			/* mt socket (Receiving) */
	char buffer[SIZE];

	unsigned char mc_ttl=1;     /* time to live (hop count) */

	const int on = 1;

	/* create socket to join multicast group on */
	if ((mtSockfd_recv = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
	    err_msg("mt_recv sock: %d %s\n", errno, strerror(errno));
	}

	/* create a rt socket */
	if( (rtSockfd = socket(AF_INET, SOCK_RAW, htons(RTPROTO))) < 0){
		err_msg("rt sock: %d %s\n", errno, strerror(errno));
	}

	/* set the socket option - IP_HDRINCL */
	if(setsockopt(rtSockfd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0){
		err_msg("set rt sock option: %d %s\n", errno, strerror(errno));
	}

	if(argc > 1){	/* Only for the source node */
		struct Tour tour;
		initTour(&tour, argc, argv); /* Create list of visiting nodes and their addresses. */
		printVisitingNode(&tour); /* Print list of visiting nodes and their addresses */

		joinMTGroup(mtSockfd_recv, MULTIADDR, MULTIPORT);

		struct sockaddr_in	dest, src;
		bzero(&src, sizeof(src));
		src.sin_family = AF_INET;

		if (inet_pton(AF_INET, tour.addrs[0].ipAddr, &src.sin_addr) <= 0){
			err_quit("inet_pton error for %s", tour.addrs[0]);
		}
		src.sin_port = htons(RTPORT);

		bzero(&dest, sizeof(dest));
		dest.sin_family = AF_INET;
		
		dest.sin_port = htons(RTPORT);
		
		if( (bind(rtSockfd, (SA *) &dest, sizeof(dest)) < 0)) {
			err_msg("bind rt sock: %d %s\n", errno, strerror(errno));
		}
		
		if (inet_pton(AF_INET, tour.addrs[tour.index].ipAddr, &src.sin_addr) <= 0){
			err_quit("inet_pton error for %s", tour.addrs[0]);
		}
		
		char *buf;
		int userlen = sizeof(struct Tour);
		buf = (char *)malloc(sizeof(struct ip) + userlen);
		memcpy(buf + sizeof(struct ip), &tour, userlen);
		fillHdr(buf, userlen, &src, &dest);

		err_msg("----------------------------------------");
		err_msg("Send RT packet from %s to %s",tour.addrs[0].ipAddr, tour.addrs[tour.index].ipAddr );
		
		if (sendto(rtSockfd, &buf, sizeof(buf), 0, (struct sockaddr *) &dest, sizeof(dest)) < 0){
			err_msg("rt socket send: %d %s\n", errno, strerror(errno));
		}
	}

	/* create a socket for sending to the multicast address */
	if ((mtSockfd_send = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		err_msg("mt sock: %d %s\n", errno, strerror(errno));
	}
	
	/* set the TTL (time to live/hop count) for the send */
	if ((setsockopt(mtSockfd_send, IPPROTO_IP, IP_MULTICAST_TTL, (void*) &mc_ttl, sizeof(mc_ttl))) < 0) {
		err_msg("set mt_send sock TTL: %d %s\n", errno, strerror(errno));
	}
	printf("Waiting\n");
	while (recv(rtSockfd, buffer, SIZE, 0) > 0){
		printf ("Caught rt packet: %s\n", buffer+sizeof(struct ip));
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
	struct IpAddress mtAddr;
	strcpy(mtAddr.ipAddr, MULTIADDR);
	tour->mtAddr = mtAddr;
}

void printVisitingNode(struct Tour *tour){
	err_msg("List of visiting nodes");
	int i;
	for(i = 0; i< tour->numNodes; i++){
		if(i == 0) err_msg("Source: vm%d (%s)", tour->nodes[i], tour->addrs[i].ipAddr);
		else err_msg("vm%d (%s)", tour->nodes[i], tour->addrs[i].ipAddr);
	}
}

void joinMTGroup(int mtSockfd_recv, char* mc_addr_str, unsigned short mc_port){
	const int on = 1;
	struct sockaddr_in mc_addr;	/* socket address structure */
	struct ip_mreq mc_req;		/* multicast request structure */

	/* set reuse port to on to allow multiple binds per host */
	if ((setsockopt(mtSockfd_recv, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) < 0) {
		err_msg("set mt_recv sock option: %d %s\n", errno, strerror(errno));
	}
	/* construct a multicast address structure */
	memset(&mc_addr, 0, sizeof(mc_addr));
	mc_addr.sin_family      = AF_INET;
	mc_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	mc_addr.sin_port        = htons(mc_port);

	/* bind to multicast address to socket */
	if ((bind(mtSockfd_recv, (struct sockaddr *) &mc_addr, sizeof(mc_addr))) < 0) {
		err_msg("bind mt_recv sock: %d %s\n", errno, strerror(errno));
	}
	
	/* construct an IGMP join request structure */
	mc_req.imr_multiaddr.s_addr = inet_addr(mc_addr_str);
	mc_req.imr_interface.s_addr = htonl(INADDR_ANY);

	/* send an ADD MEMBERSHIP message via setsockopt */
	if ((setsockopt(mtSockfd_recv, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*) &mc_req, sizeof(mc_req))) < 0) {
		err_msg("set mt_recv sock option: %d %s\n", errno, strerror(errno));
	}
}

void fillHdr(char *buf, int userlen, struct sockaddr_in *src, struct sockaddr_in *dest){
	struct ip		*ip;

	ip = (struct ip *) buf;
	bzero(ip, sizeof(*ip));

	/* 4fill in rest of IP header; */
	/* 4ip_output() calcuates & stores IP header checksum */
	ip->ip_v = 4;
	ip->ip_hl = sizeof(struct ip) >> 2;
	ip->ip_tos = 0;
	ip->ip_len = htons(userlen);	/* network byte order */
	ip->ip_id = ID;			/* let IP set this */
	ip->ip_off = 0;			/* frag offset, MF and DF flags */
	ip->ip_ttl = 64;
	ip->ip_p = RTPROTO;
	ip->ip_sum = 0;
	ip->ip_src.s_addr = ((struct sockaddr_in *) src)->sin_addr.s_addr;
	ip->ip_dst.s_addr = ((struct sockaddr_in *) dest)->sin_addr.s_addr;

}
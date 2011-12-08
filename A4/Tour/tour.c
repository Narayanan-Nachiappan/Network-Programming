#include "unp.h"
#include "a.h"
#include <string.h>
#include <linux/ip.h>
#include <linux/icmp.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <linux/ip.h>
#include <linux/icmp.h>
#include <string.h>
#include <unistd.h>
 
 
char dst_addr[15];
char src_addr[15];
 
unsigned short in_cksum(unsigned short *, int);
void parse_argvs(char**, char*, char* );
void usage();


void initTour(struct Tour*, int, char**);
void printVisitingNode(struct Tour*);
void joinMTGroup(int, char*, unsigned short);
void fillHdr(char*, int, char*, char*);
unsigned short in_cksum(unsigned short *, int);

int main(int argc, char **argv){
/*	int rtSockfd;				/* rt socket */
//	int mtSockfd_send;			/* mt socket (Sending) */
//	int mtSockfd_recv;			/* mt socket (Receiving) */
//	char buffer[SIZE];

//	unsigned char mc_ttl=1;     /* time to live (hop count) */

//	int on;
	
	/* create socket to join multicast group on */
	/*if ((mtSockfd_recv = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
	    err_msg("mt_recv sock: %d %s\n", errno, strerror(errno));
	}

	/* set the socket option - IP_HDRINCL */
	/*if(setsockopt(rtSockfd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0){
		err_msg("set rt sock option: %d %s\n", errno, strerror(errno));
	}

	if(argc > 1){	/* Only for the source node */
	/*	printVisitingNode(&tour); /* Print list of visiting nodes and their addresses */
	/*	joinMTGroup(mtSockfd_recv, MULTIADDR, MULTIPORT);

	/*	struct sockaddr_in	dest, src;
		bzero(&src, sizeof(src));
		src.sin_family = AF_INET;
		if (inet_pton(AF_INET, tour.addrs[0].ipAddr, &src.sin_addr) <= 0){
			err_quit("inet_pton error for %s", tour.addrs[0].ipAddr);
		}
		//src.sin_port = htons(RTPORT);

		bzero(&dest, sizeof(dest));
		dest.sin_family = AF_INET;
		if (inet_pton(AF_INET, tour.addrs[tour.index].ipAddr, &dest.sin_addr) <= 0){
			err_quit("inet_pton error for %s", tour.addrs[tour.index].ipAddr);
		}
		//dest.sin_port = htons(RTPORT);

		char *buf;
		int userlen = sizeof(struct Tour);

		buf = (char *)malloc(sizeof(struct iphdr) + sizeof(struct icmphdr) +  userlen);

		memcpy(buf + sizeof(struct iphdr) + sizeof(struct icmphdr), &tour, userlen);
		
		fillHdr(buf, userlen, tour.addrs[0].ipAddr, tour.addrs[tour.index].ipAddr);
		
		/* Check the payload*/
	/*	err_msg("%s", ((struct Tour*)(buf + sizeof(struct iphdr) + sizeof(struct icmphdr)  ))->addrs[0].ipAddr);

		/* Check the header */
	/*	struct iphdr *ip;
		ip = (struct iphdr *) buf;
		err_msg("%d", ip->protocol);

		err_msg("----------------------------------------");
		err_msg("Send RT packet from %s to %s",tour.addrs[0].ipAddr, tour.addrs[tour.index].ipAddr );

		//char temp[20];
		//Inet_ntop(AF_INET, &dest.sin_addr, temp, sizeof(temp));
		//err_msg("%s", temp);
		
		if (sendto(rtSockfd, &buf, sizeof(struct iphdr) + sizeof(struct icmphdr) + userlen, 0, (struct sockaddr *) &dest, sizeof(dest)) < 0){
			err_msg("rt socket send: %d %s\n", errno, strerror(errno));
		}
	}

	/* create a socket for sending to the multicast address */
	/*if ((mtSockfd_send = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		err_msg("mt sock: %d %s\n", errno, strerror(errno));
	}
	
	/* set the TTL (time to live/hop count) for the send */
	/*if ((setsockopt(mtSockfd_send, IPPROTO_IP, IP_MULTICAST_TTL, (void*) &mc_ttl, sizeof(mc_ttl))) < 0) {
		err_msg("set mt_send sock TTL: %d %s\n", errno, strerror(errno));
	}
	printf("Waiting\n");
	
	/*int n = 0;
	while (1){
		n = recvmsg(rtSockfd, &buffer, 0);
		buffer[n] = 0;
		printf ("Caught rt packet: %s\n", buffer);
	}
	*/
	/*if( recv(rtSockfd, buffer, sizeof(struct iphdr) + sizeof(struct icmphdr) + sizeof(struct Tour), 0) == -1 ){
		err_msg("recv : %d %s\n", errno, strerror(errno));
	} else {
		printf("Received\n");
	    struct iphdr *ip_reply;
		
		ip_reply = (struct iphdr*) buffer;
	    printf("ID: %d\n", ntohs(ip_reply->id));
	    printf("TTL: %d\n", ip_reply->ttl);
    }
	*/

	err_msg("CSE 533 : Network Programming");
	err_msg("Amelia Ellison - 107838108");
	err_msg("Narayanan Nachiappan - 107996031");
	err_msg("Youngbum Kim - 107387376");
	err_msg("Assignment #4");
	err_msg("----------------------------------------");
	err_msg("tour.c");
	err_msg("----------------------------------------");
    
	struct iphdr* ip;
    struct iphdr* ip_reply;
    struct icmphdr* icmp;
    struct sockaddr_in connection;
    char* packet;
    char* buffer;
    int rtSockfd;
    int optval;
    int addrlen;
 
    struct Tour tour;
	initTour(&tour, argc, argv);
	printVisitingNode(&tour); /* Print list of visiting nodes and their addresses */

	parse_argvs(argv, dst_addr, src_addr);

	strncpy(src_addr, tour.addrs[0].ipAddr, 15);
	strncpy(dst_addr, tour.addrs[tour.index].ipAddr, 15);

    printf("Source address: %s\n", src_addr);
    printf("Destination address: %s\n", dst_addr);

	/*hohoho*/

    /*
     * allocate all necessary memory
    */
    ip = malloc(sizeof(struct iphdr));
    ip_reply = malloc(sizeof(struct iphdr));
    icmp = malloc(sizeof(struct icmphdr));
    packet = malloc(sizeof(struct iphdr) + sizeof(struct icmphdr));
    buffer = malloc(sizeof(struct iphdr) + sizeof(struct icmphdr));
    /****************************************************************/
     
    ip = (struct iphdr*) packet;
    icmp = (struct icmphdr*) (packet + sizeof(struct iphdr));
     
    /*  
     *  here the ip packet is set up except checksum
     */
    ip->ihl          = 5;
    ip->version      = 4;
    ip->tos          = 0;
    ip->tot_len      = sizeof(struct iphdr) + sizeof(struct icmphdr);
    ip->id           = htons(ID);
    ip->ttl          = 255;
    ip->protocol     = RTPROTO;
    ip->saddr        = inet_addr(src_addr);
    ip->daddr        = inet_addr(dst_addr);
 
	/* create a rt socket */
	if( (rtSockfd = socket(AF_INET, SOCK_RAW, RTPROTO)) < 0){
		err_msg("rt sock: %d %s\n", errno, strerror(errno));
	}
	
    /* 
     *  IP_HDRINCL must be set on the socket so that
     *  the kernel does not attempt to automatically add
     *  a default ip header to the packet
     */
     
    setsockopt(rtSockfd, IPPROTO_IP, IP_HDRINCL, &optval, sizeof(int));
     
    /*
     *  here the icmp packet is created
     *  also the ip checksum is generated
     */
    icmp->type           = ICMP_ECHO;
    icmp->code           = 0;
    icmp->un.echo.id     = 0;
    icmp->un.echo.sequence = 0;
    icmp->checksum       = 0;
    icmp-> checksum      = in_cksum((unsigned short *)icmp, sizeof(struct icmphdr));
     
    ip->check            = in_cksum((unsigned short *)ip, sizeof(struct iphdr));
     
    connection.sin_family = AF_INET;
    connection.sin_addr.s_addr = inet_addr(dst_addr);
     
    /*
     *  now the packet is sent
     */
     
    sendto(rtSockfd, packet, ip->tot_len, 0, (struct sockaddr *)&connection, sizeof(struct sockaddr));
    printf("Sent %d byte packet to %s\n", ip->tot_len, dst_addr);
     
    /*
     *  now we listen for responses
     */
    addrlen = sizeof(connection);
    if (recv(rtSockfd, buffer, sizeof(struct iphdr) + sizeof(struct icmphdr), 0) == -1)
    {
    perror("recv");
    }
    else
    {
    printf("Received %d byte reply from %s:\n", sizeof(buffer), dst_addr);
        ip_reply = (struct iphdr*) buffer;
    printf("ID: %d\n", ntohs(ip_reply->id));
    printf("TTL: %d\n", ip_reply->ttl);
    }
    close(rtSockfd);
    return 0;



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

void fillHdr(char *buf, int userlen, char *src_addr, char *dest_addr){
	struct iphdr *ip;
	struct icmphdr *icmp;
	ip = (struct ip *) buf;
	icmp = (struct icmphdr *) (buf + sizeof(struct iphdr));
	bzero(buf, sizeof(struct iphdr) + sizeof(struct icmphdr));
	
	ip->ihl = 5;
    ip->version = 4;
    ip->tos = 0;
    ip->tot_len = sizeof(struct iphdr) + sizeof(struct icmphdr) +  userlen;
    ip->id = htons(ID);
    ip->ttl = 255;
    ip->protocol = RTPROTO;
    ip->saddr = inet_addr(src_addr);//((struct sockaddr_in *) src)->sin_addr.s_addr;
    ip->daddr = inet_addr(dest_addr);//((struct sockaddr_in *) dest)->sin_addr.s_addr;
	
	icmp->type = ICMP_ECHO;
    icmp->code = 0;
    icmp->un.echo.id = 0;
    icmp->un.echo.sequence = 0;
    icmp->checksum = 0;
	icmp-> checksum = in_cksum((unsigned short *)icmp, sizeof(struct icmphdr));
	
	ip->check = in_cksum((unsigned short *)ip, sizeof(struct iphdr));
	
	/*	struct ip	*ip;
	ip = (struct ip *) buf;
	bzero(ip, sizeof(*ip));*/

/*	ip->ip_v = 4;
	ip->ip_hl = sizeof(struct ip) >> 2;
	ip->ip_tos = 0;	
//	ip->ip_len = htons(sizeof(struct ip) + userlen);	/* network byte order */
/*	ip->ip_id = htons(ID);
	ip->ip_off = 0;
	ip->ip_ttl = 64;
	ip->ip_p = IPPROTO_RAW;
	ip->ip_src.s_addr = ((struct sockaddr_in *) src)->sin_addr.s_addr;
	ip->ip_dst.s_addr = ((struct sockaddr_in *) dest)->sin_addr.s_addr;
	ip->ip_sum = checksum(buf, sizeof(struct ip));
*/
}

unsigned short in_cksum(unsigned short *addr, int len)
{
    register int sum = 0;
    u_short answer = 0;
    register u_short *w = addr;
    register int nleft = len;
    /*
     * Our algorithm is simple, using a 32 bit accumulator (sum), we add
     * sequential 16 bit words to it, and at the end, fold back all the
     * carry bits from the top 16 bits into the lower 16 bits.
     */
    while (nleft > 1)
    {
      sum += *w++;
      nleft -= 2;
    }
    /* mop up an odd byte, if necessary */
    if (nleft == 1)
    {
      *(u_char *) (&answer) = *(u_char *) w;
      sum += answer;
    }
    /* add back carry outs from top 16 bits to low 16 bits */
    sum = (sum >> 16) + (sum & 0xffff);       /* add hi 16 to low 16 */
    sum += (sum >> 16);               /* add carry */
    answer = ~sum;              /* truncate to 16 bits */
    return (answer);
}

void parse_argvs(char** argv, char* dst, char* src)
{
    int i;
    if(!(*(argv + 1))) 
    {
    /* there are no options on the command line */
    usage();
    exit(EXIT_FAILURE); 
    }
    if (*(argv + 1) && (!(*(argv + 2)))) 
    {
    /* 
     *   only one argument provided
     *   assume it is the destination server
     *   source address is local host
     */
    strncpy(dst, *(argv + 1), 15);
    strncpy(src, getip(), 15);
    return;
    }
    else if ((*(argv + 1) && (*(argv + 2))))
    {
    /* 
     *    both the destination and source address are defined
     *    for now only implemented is a source address and 
     *    destination address
     */
    strncpy(dst, *(argv + 1), 15);
    i = 2;
    while(*(argv + i + 1))
    {
        if (strncmp(*(argv + i), "-s", 2) == 0)
        {
        strncpy(src, *(argv + i + 1), 15);
        break;
        }
        i++;
    }
 
    }
}
 
void usage()
{
    fprintf(stderr, "\nUsage: pinger [destination] <-s [source]>\n");
    fprintf(stderr, "Destination must be provided\n");
    fprintf(stderr, "Source is optional\n\n");
}
 
char* getip()
{
    char buffer[256];
    struct hostent* h;
     
    gethostname(buffer, 256);
    h = gethostbyname(buffer);
     
    return inet_ntoa(*(struct in_addr *)h->h_addr);
     
}
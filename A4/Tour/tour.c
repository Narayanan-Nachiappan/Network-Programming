#include "unp.h"
#include "a.h"
#include <string.h>
#include <linux/ip.h>
#include <time.h>

#define MAX_LEN  1024   /* maximum receive string size */

char dst_addr[15];
char src_addr[15];

int mtRecv(char*, int);
void initTour(struct Tour*, int, char**);
void printVisitingNode(struct Tour*);
unsigned short in_cksum(unsigned short *, int);

int main(int argc, char **argv){
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
    struct sockaddr_in connection;
    char* packet;
    char* buffer;
	int rtFlag = 0;		/* rt packet visited? */
    int rtSockfd;		/* rt socket */
	int mtSockfd_send;	/* mt socket (Sending) */
	int mtSockfd_recv;	/* mt socket (Receiving) */
    int optval;
    int addrlen;
	unsigned char mc_ttl=1;     /* time to live (hop count) */
	char *send_str = (char *)malloc(128);	/* string to send */
	char *mtBuffer = (char *)malloc(128);	/* buffer to receive string */
	/* create a socket for sending to the multicast address */
	if ((mtSockfd_send = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		err_msg("mt send sock: %d %s\n", errno, strerror(errno));
	}

	/* set the TTL (time to live/hop count) for the send */
	if ((setsockopt(mtSockfd_send, IPPROTO_IP, IP_MULTICAST_TTL, (void*) &mc_ttl, sizeof(mc_ttl))) < 0) {
		err_msg("mt send sock option: %d %s\n", errno, strerror(errno));
	}

	/* create a rt socket */
	if( (rtSockfd = socket(AF_INET, SOCK_RAW, RTPROTO)) < 0){
		err_msg("rt sock: %d %s\n", errno, strerror(errno));
	}

	/* set the socket option - IP_HDRINCL */
	if(setsockopt(rtSockfd, IPPROTO_IP, IP_HDRINCL, &optval, sizeof(int)) < 0){
		err_msg("set rt sock option: %d %s\n", errno, strerror(errno));
	}

	/* allocate all necessary memory */
	ip_reply = malloc(sizeof(struct iphdr));
    struct Tour *receivedTour = malloc(sizeof(struct Tour));
	packet = malloc(sizeof(struct iphdr) + sizeof(struct Tour));
    buffer = malloc(sizeof(struct iphdr) + sizeof(struct Tour));
    /****************************************************************/

    if(argc > 1){	/* Only for the source node */
		mtSockfd_recv = mtRecv(MULTIADDR, MULTIPORT);
		struct Tour tour;
		initTour(&tour, argc, argv);
		printVisitingNode(&tour); /* Print list of visiting nodes and their addresses */

		strncpy(dst_addr, getip(), 15);
	    strncpy(src_addr, getip(), 15);
		strncpy(src_addr, tour.addrs[0].ipAddr, 15);
		strncpy(dst_addr, tour.addrs[tour.index].ipAddr, 15);	
		
		printf("Source address: %s\n", src_addr);
		printf("Destination address: %s\n", dst_addr);

		ip = malloc(sizeof(struct iphdr));
		ip = (struct iphdr*) packet;
     
		/* ip header */
		ip->ihl          = 5;
		ip->version      = 4;
		ip->tos          = 0;
		ip->tot_len      = sizeof(struct iphdr)  + sizeof(struct Tour);
		ip->id           = htons(ID);
		ip->ttl          = 255;
		ip->protocol     = RTPROTO;
		ip->saddr        = inet_addr(src_addr);
		ip->daddr        = inet_addr(dst_addr);
		
		/* Copy the Tour List to the packet */
		memcpy(packet + sizeof(struct iphdr), &tour, sizeof(struct Tour));
		ip->check = in_cksum((unsigned short *)ip, sizeof(struct iphdr) + sizeof(struct Tour));

		connection.sin_family = AF_INET;
		connection.sin_addr.s_addr = inet_addr(dst_addr);

		err_msg("----------------------------------------");
		err_msg("Send RT packet from %s to %s",src_addr, dst_addr);

		if (sendto(rtSockfd, packet, ip->tot_len, 0, (struct sockaddr *)&connection, sizeof(struct sockaddr)) < 0){
			err_msg("rt socket send: %d %s\n", errno, strerror(errno));
		}

		int recv_len;                 /* length of string received */
		struct sockaddr_in from_addr; /* packet source */
		unsigned int from_len;        /* source addr length */

		/* Receiving MC Packets */
		for (;;) {	/* loop forever */
			/* clear the receive buffers & structs */	
			memset(mtBuffer, 0, sizeof(mtBuffer));
			from_len = sizeof(from_addr);
			memset(&from_addr, 0, from_len);

			/* block waiting to receive a packet */
			if ((recv_len = recvfrom(mtSockfd_recv, mtBuffer, 128, 0, (struct sockaddr*)&from_addr, &from_len)) < 0) {
				err_msg("MC Packet receive: %d %s\n", errno, strerror(errno));
			}
			err_msg("----------------------------------------");
			/* output received string */
			err_msg("Node vm%d. Received:", tour.nodes[0]);
			err_msg("Msg: %s", mtBuffer);
		}
		
	} else { /* Not the Source code */
		err_msg("Waiting for the Packets");
		addrlen = sizeof(connection);
		/*
			rt
			pg
			mc

		*/
		fd_set reads, temps;
		int fd_max, result;

		for ( ; ; ) {
			FD_ZERO(&reads); // initialize to 0;
			FD_SET(rtSockfd, &reads);
			//FD_SET(pgSockfd, &reads);
			FD_SET(mtSockfd_recv, &reads);
			fd_max = max(rtSockfd, mtSockfd_recv) + 1;
			
			int fd;
			temps = reads;
			result = select(fd_max, &reads, 0, 0, 0);
			if(FD_ISSET(rtSockfd, &reads)){
				if(recv(rtSockfd, buffer, sizeof(struct iphdr) + sizeof(struct Tour), 0) >= 0){
					err_msg("----------------------------------------");
					err_msg("Packet received - check the ID value");
					ip_reply = (struct iphdr*) buffer;
					/* check received packet */
					receivedTour = (struct Tour*) (buffer + sizeof(struct iphdr));
					err_msg("----------------------------------------");
					err_msg("Protocol: %d", ip_reply->protocol);
					err_msg("ID: %d", ntohs(ip_reply->id));
					err_msg("TTL: %d", ip_reply->ttl);
					if(ntohs(ip_reply->id) == ID){
						err_msg("[Valid ID] - proceed");
						struct sockaddr_in servaddr;
						struct hostent *hptr;
						struct in_addr **pptr;
						servaddr.sin_addr.s_addr = ip_reply->saddr;
						pptr = &servaddr.sin_addr;
							
						if ( (hptr = gethostbyaddr(pptr, sizeof (pptr), AF_INET)) == NULL) {
							err_sys("gethostbyaddr error for host: %s: %s",	ip_reply, hstrerror(h_errno));
						}
			
						time_t ticks;
						ticks = time(NULL);
						err_msg("%.24s\r", ctime(&ticks));
						err_msg(" received source routing packet from < %s >", hptr->h_name);
						err_msg("----------------------------------------");
						err_msg("List of visiting nodes");
						err_msg("Tour information");
						err_msg("Number of Nodes: %d", receivedTour->numNodes);
						err_msg("Index: %d", receivedTour->index);
						printVisitingNode(receivedTour);
						err_msg("MC Addr: %s", receivedTour->mtAddr.ipAddr);
						err_msg("MC Port: %d", receivedTour->mtPort);
			
						if(rtFlag == 0){
							err_msg("----------------------------------------");
							err_msg("The first time rt packet visited - join the MC group");
							mtSockfd_recv = mtRecv(MULTIADDR, MULTIPORT);
							//mtSockfd_recv = mtRecv(receivedTour->mtAddr.ipAddr, receivedTour->mtPort);
					
							rtFlag = 1;
							err_msg("----------------------------------------");
							err_msg("Send ping to the Source node");

							/**
					


							**/

							err_msg("----------------------------------------");
							if(receivedTour->index+1 == receivedTour->numNodes){
								err_msg("Reached the last node!");
								struct sockaddr_in mc_addr; /* socket address structure */
								unsigned int send_len;      /* length of string to send */
								unsigned char mc_ttl=1;     /* time to live (hop count) */
  
								/* construct a multicast address structure */
								memset(&mc_addr, 0, sizeof(mc_addr));
								mc_addr.sin_family      = AF_INET;
								mc_addr.sin_addr.s_addr = inet_addr(receivedTour->mtAddr.ipAddr);
								mc_addr.sin_port        = htons(receivedTour->mtPort);

								memset(send_str, 0, sizeof(send_str));	/* clear send buffer */
								sprintf(send_str, "This is node vm%d. Tour has ended. Group members please identify yourselves.\n\0", receivedTour->nodes[receivedTour->index]);
								send_len = strlen(send_str);

								err_msg("----------------------------------------");
								err_msg("Node vm%d. Sending:",receivedTour->nodes[receivedTour->index]);
								err_msg("Msg: %s", send_str);
								err_msg("----------------------------------------");

								/* send string to multicast address */
								if ((sendto(mtSockfd_send, send_str, send_len, 0, (struct sockaddr *) &mc_addr, sizeof(mc_addr))) != send_len) {
									perror("sendto() sent incorrect number of bytes");
									exit(1);
								}
								memset(send_str, 0, sizeof(send_str));
							} else {
								err_msg("Send RT packet from %s to %s"
									,receivedTour->addrs[receivedTour->index].ipAddr, receivedTour->addrs[receivedTour->index+1].ipAddr);
								ip_reply->tot_len = sizeof(struct iphdr)  + sizeof(struct Tour);
								ip_reply->saddr = inet_addr(receivedTour->addrs[receivedTour->index].ipAddr);
								receivedTour->index = receivedTour->index + 1;
								ip_reply->daddr = inet_addr(receivedTour->addrs[receivedTour->index].ipAddr);
								ip_reply->check = in_cksum((unsigned short *)ip_reply, sizeof(struct iphdr) + sizeof(struct Tour));

								connection.sin_family = AF_INET;
								connection.sin_addr.s_addr = inet_addr(receivedTour->addrs[receivedTour->index].ipAddr);

								if (sendto(rtSockfd, buffer, ip_reply->tot_len, 0, (struct sockaddr *)&connection, sizeof(struct sockaddr)) < 0){
									err_msg("rt socket send: %d %s\n", errno, strerror(errno));
								}
							}
						}
					} else {
						err_msg("[Invalid ID] - ignore");
						continue;
					}
				}
			}
			if(FD_ISSET(mtSockfd_recv, &reads)){ // mt socket
				int recv_len;                 /* length of string received */
				struct sockaddr_in from_addr; /* packet source */
				unsigned int from_len;        /* source addr length */
				/* clear the receive buffers & structs */	
					memset(mtBuffer, 0, sizeof(mtBuffer));
					from_len = sizeof(from_addr);
					memset(&from_addr, 0, from_len);

				/* block waiting to receive a packet */
					if ((recv_len = recvfrom(mtSockfd_recv, mtBuffer, 128, 0, (struct sockaddr*)&from_addr, &from_len)) < 0) {
						err_msg("MC Packet receive: %d %s\n", errno, strerror(errno));
					}
					err_msg("----------------------------------------");
				/* output received string */
					err_msg("Node vm%d. Received:");//, tour.nodes[0]);
					err_msg("Msg: %s", mtBuffer);
			}
		}
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
	memcpy(tour->nodes, nodes, sizeof(int) * 20);
	memcpy(tour->addrs, ipAddrs, sizeof(struct IpAddress) * 20);
	tour->index = 1;
	struct IpAddress mtAddr;
	strcpy(mtAddr.ipAddr, MULTIADDR);
	tour->mtAddr = mtAddr;
	tour->mtPort = MULTIPORT;
}

void printVisitingNode(struct Tour *tour){
	err_msg("List of visiting nodes");
	int i;
	for(i = 0; i< tour->numNodes; i++){
		if(i == 0) err_msg("Source: vm%d (%s)", tour->nodes[i], tour->addrs[i].ipAddr);
		else err_msg("vm%d (%s)", tour->nodes[i], tour->addrs[i].ipAddr);
	}
}

unsigned short in_cksum(unsigned short *addr, int len){
    register int sum = 0;
    u_short answer = 0;
    register u_short *w = addr;
    register int nleft = len;
    while (nleft > 1){
      sum += *w++;
      nleft -= 2;
    }
    if (nleft == 1)
    {
      *(u_char *) (&answer) = *(u_char *) w;
      sum += answer;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = ~sum;
    return (answer);
}
 
char* getip()
{
    char buffer[256];
    struct hostent* h;
     
    gethostname(buffer, 256);
    h = gethostbyname(buffer);
     
    return inet_ntoa(*(struct in_addr *)h->h_addr);
     
}

int mtRecv(char* mc_addr_str, int mc_port) {

  int sock;                     /* socket descriptor */
  int flag_on = 1;              /* socket option flag */
  struct sockaddr_in mc_addr;   /* socket address structure */
  char recv_str[MAX_LEN+1];     /* buffer to receive string */
  int recv_len;                 /* length of string received */
  struct ip_mreq mc_req;        /* multicast request structure */
  struct sockaddr_in from_addr; /* packet source */
  unsigned int from_len;        /* source addr length */

  /* create socket to join multicast group on */
  if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    perror("socket() failed");
    exit(1);
  }
  
  /* set reuse port to on to allow multiple binds per host */
  if ((setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &flag_on,
       sizeof(flag_on))) < 0) {
    perror("setsockopt() failed");
    exit(1);
  }
  
  /* construct a multicast address structure */
  memset(&mc_addr, 0, sizeof(mc_addr));
  mc_addr.sin_family      = AF_INET;
  mc_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  mc_addr.sin_port        = htons(mc_port);

  /* bind to multicast address to socket */
  if ((bind(sock, (struct sockaddr *) &mc_addr, 
       sizeof(mc_addr))) < 0) {
    perror("bind() failed");
    exit(1);
  }

  /* construct an IGMP join request structure */
  mc_req.imr_multiaddr.s_addr = inet_addr(mc_addr_str);
  mc_req.imr_interface.s_addr = htonl(INADDR_ANY);

  /* send an ADD MEMBERSHIP message via setsockopt */
  if ((setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, 
       (void*) &mc_req, sizeof(mc_req))) < 0) {
    perror("setsockopt() failed");
    exit(1);
  }

  return sock;
}
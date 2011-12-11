#include "unp.h"
#include "a.h"
#include <string.h>
#include <linux/ip.h>
#include <time.h>

int ping = 0;

#define MAX_LEN  1024   /* maximum receive string size */

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
    
	struct iphdr* ip;	/* ip header */
    struct iphdr* ip_reply;
    struct sockaddr_in connection;
    char* packet;
    char* buffer;
	int rtFlag = 0;		/* rt packet visited? */
	int mtFlag = 0;
    int rtSockfd;		/* rt socket */
	int mtSockfd_send;	/* mt socket (Sending) */
	int mtSockfd_recv;	/* mt socket (Receiving) */
    int request;        /* packet socket*/
	int pg;				/* pg socket*/
	int optval = 1;
    int addrlen;
	unsigned char mc_ttl=1;     /* time to live (hop count) */
	char *send_str = (char *)malloc(128);	/* string to send */
	char *mtBuffer = (char *)malloc(128);	/* buffer to receive string */

	/* create a PF_PACKET socket for sending pings */
	request = socket(PF_PACKET, SOCK_RAW, htons(PROTO_TYPE));
	if(request < 0){
		err_msg("request sock: %d %s\n", errno, strerror(errno));
	} 
	
	/* pg socket is used to receive ICMP_ECHOREPLY */
	pg = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if(pg < 0){
		err_msg("pg sock: %d %s\n", errno, strerror(errno));
	}
	
	if(setsockopt(pg, IPPROTO_IP, IP_HDRINCL, &optval, sizeof(int)) < 0){
		err_msg("set pg sock option: %d %s\n", errno, strerror(errno));
	}
	
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
		mtSockfd_recv = mtRecv(MULTIADDR, MULTIPORT); /* Set up the mt socket (receiving) and Join the Multicast Group */
		struct Tour tour;
		char src_addr[15], dst_addr[15];
		initTour(&tour, argc, argv);	/* Initialize Tour List */
		printVisitingNode(&tour);		/* Print list of visiting nodes and their addresses */

		strncpy(src_addr, tour.addrs[0].ipAddr, 15);
		strncpy(dst_addr, tour.addrs[tour.index].ipAddr, 15);

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

		fd_set reads, temps , rset;
		int fd_max, result;
		
		for ( ; ; ) {
			FD_ZERO(&reads); // initialize to 0;
			FD_SET(mtSockfd_recv, &reads);
			FD_SET(request, &reads);
			fd_max = max(request, mtSockfd_recv) + 1;

			result = select(fd_max, &reads, 0, 0, 0);

			if(FD_ISSET(request, &reads)) //got a ping request, send ping reply //
			{
				//ping_send2(request, pg);
			}
			if(FD_ISSET(mtSockfd_recv, &reads)){
				/* Receiving MC Packets */
				/* clear the receive buffers & structs */	
				memset(mtBuffer, 0, 128);
				from_len = sizeof(from_addr);
				memset(&from_addr, 0, from_len);

				/* block waiting to receive a packet */
				if ((recv_len = recvfrom(mtSockfd_recv, mtBuffer, 128, 0, (struct sockaddr*)&from_addr, &from_len)) < 0) {
					err_msg("MC Packet receive: %d %s\n", errno, strerror(errno));
				}
				err_msg("----------------------------------------");	/* output received string */
				err_msg("Node vm%d. Received:", tour.nodes[0]);
				err_msg("Msg: %s", mtBuffer);

				if(mtFlag == 0){
					struct sockaddr_in mc_addr; /* socket address structure */
					unsigned int send_len;      /* length of string to send */
					unsigned char mc_ttl=1;     /* time to live (hop count) */
  
					/* construct a multicast address structure */
					memset(&mc_addr, 0, sizeof(mc_addr));
					mc_addr.sin_family      = AF_INET;
					mc_addr.sin_addr.s_addr = inet_addr(MULTIADDR);
					mc_addr.sin_port        = htons(MULTIPORT);

					memset(send_str, 0, sizeof(send_str));	/* clear send buffer */
					sprintf(send_str, "Node vm%d. I am a member of a group.\n\0", tour.nodes[0]);
					send_len = strlen(send_str);
						
					err_msg("----------------------------------------");
					err_msg("Node vm%d Sending:", tour.nodes[0]);
					err_msg("Msg: %s", send_str);
					err_msg("----------------------------------------");

					/* send string to multicast address */
					if ((sendto(mtSockfd_send, send_str, send_len, 0, (struct sockaddr *) &mc_addr, sizeof(mc_addr))) != send_len) {
						perror("sendto() sent incorrect number of bytes");
						exit(1);
					}
					memset(send_str, 0, sizeof(send_str));
					mtFlag = 1;
				
					/*for(;;){    
						struct timeval    tv;
						tv.tv_sec = 5;
						tv.tv_usec = 0;
    
						FD_ZERO(&rset);
						FD_SET(mtSockfd_recv, &rset);
						int m = mtSockfd_recv + 1;
						select(m, NULL, &rset, NULL, &tv);
						if(FD_ISSET(mtSockfd_recv,&rset)){
							memset(mtBuffer, 0, 128);
							from_len = sizeof(from_addr);
							memset(&from_addr, 0, from_len);

							/* block waiting to receive a packet */
					/*		if ((recv_len = recvfrom(mtSockfd_recv, mtBuffer, 128, 0, (struct sockaddr*)&from_addr, &from_len)) < 0) {
								err_msg("MC Packet receive: %d %s\n", errno, strerror(errno));
							}
							err_msg("----------------------------------------");	/* output received string */
					/*		err_msg("Node vm%d. Received:", tour.nodes[0]);
							err_msg("Msg: %s", mtBuffer);
					    } else {
							err_msg("Termination tour application\n");
							close(mtSockfd_recv);
							exit(0);
						}
					}*/
				}
			}
		}
	} else { /* Not the Source code */
		err_msg("Waiting for the Packets");
		addrlen = sizeof(connection);
	
		fd_set reads, rset;
		int fd_max, result;
		time_t tick = time(NULL);
		char sourceAddr[16], thisAddr[16], src_mac[6];
		struct hwa_info	*hwahead = (struct hwa_info*)malloc(sizeof(struct hwa_info));
		struct hwa_info	*hwa = (struct hwa_info*)malloc(sizeof(struct hwa_info));
		for ( hwahead = hwa = Get_hw_addrs(); hwa != NULL; hwa = hwa->hwa_next)
		{
			if(strncmp(hwa->if_name, "eth0", 4) == 0)
			{
				memcpy((void*)src_mac, hwa->if_haddr, 6);
				break;
			}
		}
		printHW(src_mac);
		
		char *hostName = (char *)malloc(20);
		gethostname(hostName, 20);

		if ((mtSockfd_recv = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
			 perror("socket() failed");
			exit(1);
		}
		for ( ; ; ) {
			FD_ZERO(&reads); // initialize to 0;
			FD_SET(rtSockfd, &reads);
			FD_SET(mtSockfd_recv, &reads);
			FD_SET(pg, &reads);
			FD_SET(request, &reads);
			fd_max = max(rtSockfd, mtSockfd_recv);
			fd_max = max(fd_max, pg);
			fd_max = max(fd_max, request) + 1;
			
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
							//mtSockfd_recv = mtRecv(MULTIADDR, MULTIPORT);
							strcpy(sourceAddr, receivedTour->addrs[0].ipAddr);
							close(mtSockfd_recv);
							mtSockfd_recv = mtRecv(receivedTour->mtAddr.ipAddr, receivedTour->mtPort);
							
							ping = 1; /* Start pinging the source */
							rtFlag = 1;
							err_msg("----------------------------------------");
							
							if(receivedTour->index+1 == receivedTour->numNodes){
								err_msg("Reached the last node!");
								struct sockaddr_in mc_addr; /* socket address structure */
								unsigned int send_len;      /* length of string to send */
								unsigned char mc_ttl=1;     /* time to live (hop count) */
  
								int i;
								for(i = 0; i < 5; i++){
									struct hostent *hptr;
									struct in_addr **pptr;
									char ipAddr[16];
									
									hptr = gethostbyname(hostName);
									pptr = (struct in_addr**) hptr->h_addr_list;
									Inet_ntop(hptr->h_addrtype, *pptr, ipAddr, sizeof(ipAddr));

									struct sockaddr * ipaddr;
									ipaddr=(struct sockaddr *) malloc(sizeof(struct sockaddr));
									ipaddr->sa_family=1;
									strcpy(ipaddr->sa_data, sourceAddr); // Source node!!!
									err_msg("%s",ipaddr->sa_data);
									socklen_t len=sizeof(struct sockaddr);
									struct hwaddr  *haddr=malloc(sizeof(struct hwaddr));
									haddr->sll_ifindex=2;
									haddr->sll_hatype=1;
									haddr->sll_halen='6';
									//int ret_val=areq(ipaddr,len,haddr);
									//err_msg("retval %d" , ret_val);	
									
									ping_send(pg, ipAddr, sourceAddr, src_mac, haddr->sll_addr, haddr->sll_ifindex);
									ping_recv(pg);
									sleep(1);
								};

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

							/*	for(;;){    
								struct timeval    tv;
								tv.tv_sec = 5;
								tv.tv_usec = 0;
    
								FD_ZERO(&rset);
								FD_SET(mtSockfd_recv, &rset);
								int m = mtSockfd_recv + 1;
								select(m, NULL, &rset, NULL, &tv);
								if(FD_ISSET(mtSockfd_recv,&rset)){
									int recv_len;                 /* length of string received */
							/*		struct sockaddr_in from_addr; /* packet source */
							/*		unsigned int from_len;        /* source addr length */
							/*		struct sockaddr_in mc_addr; /* socket address structure */
							/*		unsigned int send_len;      /* length of string to send */
							/*		unsigned char mc_ttl=1;     /* time to live (hop count) */
							/*		memset(mtBuffer, 0, 128);
									from_len = sizeof(from_addr);
									memset(&from_addr, 0, from_len);

									/* block waiting to receive a packet */
							/*		if ((recv_len = recvfrom(mtSockfd_recv, mtBuffer, 128, 0, (struct sockaddr*)&from_addr, &from_len)) < 0) {
										err_msg("MC Packet receive: %d %s\n", errno, strerror(errno));
									}
									err_msg("----------------------------------------");
									/* output received string */
							/*		err_msg("Node %s. Received:", hostName);
									err_msg("Msg: %s", mtBuffer);
								} else {
									err_msg("Termination tour application\n");
									close(mtSockfd_recv);
									exit(0);
								}
							}*/

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
					memset(mtBuffer, 0, 128);
					from_len = sizeof(from_addr);
					memset(&from_addr, 0, from_len);

				/* block waiting to receive a packet */
					if ((recv_len = recvfrom(mtSockfd_recv, mtBuffer, 128, 0, (struct sockaddr*)&from_addr, &from_len)) < 0) {
						err_msg("MC Packet receive: %d %s\n", errno, strerror(errno));
					}
					err_msg("----------------------------------------");
				/* output received string */
					err_msg("Node %s. Received:", hostName);
					err_msg("Msg: %s", mtBuffer);

					if(mtFlag == 0){
						struct sockaddr_in mc_addr; /* socket address structure */
						unsigned int send_len;      /* length of string to send */
						unsigned char mc_ttl=1;     /* time to live (hop count) */
  
						/* construct a multicast address structure */
						memset(&mc_addr, 0, sizeof(mc_addr));
						mc_addr.sin_family      = AF_INET;
						mc_addr.sin_addr.s_addr = inet_addr(receivedTour->mtAddr.ipAddr);
						mc_addr.sin_port        = htons(receivedTour->mtPort);

						memset(send_str, 0, sizeof(send_str));	/* clear send buffer */
						sprintf(send_str, "Node %s. I am a member of a group.\n\0", hostName);
						send_len = strlen(send_str);
						
						err_msg("----------------------------------------");
						err_msg("Node %s Sending:", hostName);
						err_msg("Msg: %s", send_str);
						err_msg("----------------------------------------");

						/* send string to multicast address */
						if ((sendto(mtSockfd_send, send_str, send_len, 0, (struct sockaddr *) &mc_addr, sizeof(mc_addr))) != send_len) {
							perror("sendto() sent incorrect number of bytes");
							exit(1);
						}
						memset(send_str, 0, sizeof(send_str));
						mtFlag = 1;
					}
			}
			if(FD_ISSET(pg, &reads)){ //get ping reply
				ping_recv(pg);
			}

			if(ping == 1 && ( difftime(tick, time(NULL)) >= 1 )  ){
				err_msg("----------------------------------------");
				err_msg("Send ping to the Source Node");
				struct hostent *hptr;
				struct in_addr **pptr;
				char ipAddr[16];

				hptr = gethostbyname(hostName);
				pptr = (struct in_addr**) hptr->h_addr_list;
				Inet_ntop(hptr->h_addrtype, *pptr, ipAddr, sizeof(ipAddr));

				struct sockaddr * ipaddr;
				ipaddr=(struct sockaddr *) malloc(sizeof(struct sockaddr));
				ipaddr->sa_family=1;
				strcpy(ipaddr->sa_data, sourceAddr); // Source node!!!
				err_msg("%s",ipaddr->sa_data);
				socklen_t len=sizeof(struct sockaddr);
				struct hwaddr  *haddr=malloc(sizeof(struct hwaddr));
				haddr->sll_ifindex=2;
				haddr->sll_hatype=1;
				haddr->sll_halen='6';
				//int ret_val=areq(ipaddr,len,haddr);
				//err_msg("retval %d" , ret_val);	

				printHW(haddr->sll_addr);
				
				ping_send(pg, ipAddr, sourceAddr, src_mac, haddr->sll_addr, haddr->sll_ifindex);

				tick = time(NULL);
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
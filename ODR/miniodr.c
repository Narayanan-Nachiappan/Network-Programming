#include "odr.h"

struct interface
{
	char    if_haddr[6];	/* hardware address */
	int     if_index;		/* interface index */
	char	ip_addr[16];	/* IP address */
};


struct payload
{
	int 	srcport;
	int 	destport;
	int		msgsz;
	char	message[100];
};


struct ODRmsg
{
	int				type;
	char			src_ip[16];
	char			dest_ip[16];
	int 			broadcast_id;
	int 			hopcount;
	int				RREPsent;
	int				forced_discovery;
	struct	payload	app;
};

struct interface		interfaces[MAX_INTERFACES];
struct ODRmsg			*msg;
int 					pfsock, appsock, if_nums = 0, rtablesize = 0, dtablesize = 0, broadcast_id = 0;
char					sunpath_root[20], canonical[16];

int main(int argc, char **argv)
{
	struct msghdr  			*msg;
	struct hwa_info			*hwa, *hwahead;
	struct sockaddr_un 		su;
	struct sockaddr			*sa;
	socklen_t				addrlen;
	int 					i, staleness;
	int 					protocol, maxfdp1;
	fd_set					rset;
	char					address[16], name[16];
	char					rcvline[MAXLINE];
	struct hostent			*host;
	
	if(argc < 2)
	{
		printf("Usage: odr staleness\n");
		return -1;
	}
	
	staleness  = atoi(argv[1]);
	host = gethostname(canonical, 16);
	//host = gethostbyname(name);
	if(host == NULL)
		printf("host is NULL\n");
	printf("Local ODR on host %s\n", canonical);
	//printf("%s\n", host->h_addr);
	//strncpy(canonical, host->h_addr, 16);
	
	printf("Finding interfaces\n");
	for (i = 0, hwahead = hwa = Get_hw_addrs(); hwa != NULL && i < MAX_INTERFACES; hwa = hwa->hwa_next, i++)
	{
		/*
		sa = hwa->ip_addr;
		printf("IP addr = %s\n", sock_ntop_host(sa, sizeof(*sa)));
		//inet_ntop(AF_INET, s->sin_addr, address, sizeof(struct sockaddr_in));
		//printf("IP Address: %s\n",  address);
		printf("Index: %d\n", hwa->if_index);*/
		//printf("Hardware Address: %s\n",  hwa->if_haddr);
		//sa = interfaces[i].ip_addr;
		//inet_ntop(AF_INET, sa, address, sizeof(*sa));
		//strncpy(address, sock_ntop_host(sa, sizeof(*sa)), 16);
		strncpy(interfaces[i].if_haddr, hwa->if_haddr, 6);
		interfaces[i].if_index = hwa->if_index;
		strncpy(interfaces[i].ip_addr, hwa->ip_addr, 16);
		printf("IP Address: %s\n",  interfaces[i].ip_addr);
		printf("Hardware Address: %s\n", interfaces[i].if_haddr);
		printf("Index: %d\n", interfaces[i].if_index);
	}
	
	if_nums = i;
	printf("Found %d interfaces\n", if_nums);
	
	//Creating packet socket
	pfsock = socket(PF_PACKET, SOCK_RAW, htons(ODR_PROTOCOL));
	if(pfsock < 0)
	{
		printf("pfsock: %d %s\n", errno, strerror(errno));
		return -1;
	}
	
	//Creating application socket
	su.sun_family = AF_LOCAL;
	strcpy(su.sun_path, ODR_SUNPATH);
	strcat(su.sun_path, "\0");
	
	appsock = socket(AF_INET, SOCK_DGRAM, 0);
	if(appsock < 0)
	{
		printf("appsock: %d %s\n", errno, strerror(errno));
		return -1;
	}
	
	unlink(ODR_SUNPATH);
	if(bind(appsock, (struct sockaddr*) &su, SUN_LEN(&su)) < 0)
	{
		printf("bind failed: %d %s\n", errno, strerror(errno));
		return -1;
	}

	printf("Bound appsock\n");
	for ( ; ; ) 
	{
		printf("in for\n");
		FD_ZERO(&rset);
		printf("zero\n");
		FD_SET(pfsock, &rset);
		printf("set1\n");
		FD_SET(appsock, &rset);
		printf("set2\n");
		maxfdp1 = max(pfsock, appsock) + 1;
		printf("max\n");
		select(maxfdp1, &rset, NULL, NULL, NULL);
		
		printf("Selecting...\n");
		if(FD_ISSET(appsock, &rset))
		{
			printf("appsock is ready\n");
			//msg_recv(appsock, message, address, port);
			addrlen = sizeof(sa);
			
			if(recvfrom(appsock, rcvline, MAXLINE, 0, sa, &addrlen) < 0)
			{
					printf("appsock recvfrom error: %d %s\n", errno, strerror(errno));
					return -1;
			}
			
			//su = (struct sockaddr_un*) sa;			
			processAPPmsg(rcvline);
		}
		if(FD_ISSET(pfsock, &rset))
		{
			printf("pfsock is ready\n");
			//msg_recv(pfsock, message, address, port);
			
			if(recvmsg(pfsock, msg, 0) < 0)
			{
					printf("pfsock recvmsg error: %d %s\n", errno, strerror(errno));
					return -1;
			}
			processODRmsg(msg);
		}
	}
}


int processAPPmsg(char *rcvline)
{
	//int sockfd, char* address, int destport, char* message, int flag
	char temp[MAXLINE], message[100], address[16];
	socklen_t* address_len;
	int gr, s, port, flag;
	struct ODRmsg app;
	
	strncpy(address, strtok(rcvline, " "), 16);
	strcpy(temp, strtok(NULL, " "));
	sscanf(temp, "%d", port);
	strcpy(message, strtok(NULL, " "));
	strcpy(temp, strtok(NULL, " "));
	sscanf(temp, "%d", flag);
	
	printf("Received msg_send request from local application:\n");
	printf("Destination address: %s\n", address);
	printf("Destination port: %d\n", port);
	printf("Destination message: %s\n", message);
	printf("Flag: %d\n", flag);
	
	app.type = htons(2);
	strncpy(app.src_ip, canonical, 16);
	strncpy(app.dest_ip, address, 16);
	app.hopcount = htons(0);
	
	sendAPPmsg(&app);
	return 0;
}

int sendAPPmsg(struct ODRmsg *app)
{
	int i;
	struct sockaddr_ll socket_address;
	
	socket_address.sll_family = AF_PACKET;
	socket_address.sll_protocol = htons(ODR_PROTOCOL);
	//socket_address.sll_pkttype = PACKET_BROADCAST;
	socket_address.sll_halen = 8;
	
	//Set sll_addr to broadcast address
	for(i = 0; i < 6; i++)
	{
		socket_address.sll_addr[i] = 0xFF;
	}
	socket_address.sll_addr[6]  = 0x00;/*not used*/
	socket_address.sll_addr[7]  = 0x00;/*not used*/
	
	if(sendto(pfsock, app, sizeof(struct ODRmsg), 0, (struct sockaddr*) &socket_address, sizeof(struct sockaddr)) < 0)
	{
		printf("sendAPPmessage sendto error: %d %s\n", errno, strerror(errno));
		return -1;
	}
}

int processODRmsg(struct ODRmsg app)
{
	struct sockaddr_un su;
	char sendline[100];
	char port[6];
	if(app.type == 2 && strncmp(app.src_ip, canonical, 16) != 0)
	{	
		strncat(sendline, app.app.message, strlen(app.app.message));
		strcat(sendline, " ");
		sprintf(port, "%d", app.app.srcport);
		strcat(sendline, port);
		strcat(sendline, " ");
		strncat(sendline, app.src_ip, strlen(app.src_ip));
		strcat(sendline, "\0");
	}
	
	su.sun_family = AF_LOCAL;
	strcpy(su.sun_path, TIME_SERV_SUNPATH);
	strcat(su.sun_path, "\0");
	
	if(sendto(appsock, &app, sizeof(struct ODRmsg), 0, (struct sockaddr*) &su, sizeof(struct sockaddr)) < 0)
	{
		printf("processODRmessage sendto error: %d %s\n", errno, strerror(errno));
		return -1;
	}
}
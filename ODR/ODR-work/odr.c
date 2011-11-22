#include "odr.h"
#define MAX_UNSENT 10

struct unsentmsg
{
	struct ODRmsg msg;
	int empty;
};

struct interface		interfaces[MAX_INTERFACES];
int 					pfsock, appsock, if_nums = 0, broadcast_id = 0, staleness, unsent_num = 0;
char					sunpath_root[20], canonical[16], neighbor[6], name[16];
struct unsentmsg		unsent[MAX_UNSENT];

int processAPPmsg(char *rcvline, struct sockaddr* sa);
int processODRmsg(struct ODRmsg *m, struct sockaddr* sa);
int waitforRREP(char *dest);
int sendAPPmsg(struct ODRmsg app, int route);
int sendRREQ(struct ODRmsg RREQ, int out_if, int route, int force);
int sendRREP(struct ODRmsg RREP, int out_if, int route, int force);
int sendPacket(struct ODRmsg msg, int out_if, int rte, int broadcast);
int toss(char *rcvline);
int sendUnsentMsgs();
int APPBackup(struct ODRmsg app);
void unsent_init();
int isDestination(char *address);
int findInterface(int if_index);
void printHW(char* ptr);
void printAPPmsg(struct ODRmsg msg);

int main(int argc, char **argv)
{
	struct ODRmsg			*msg;
	struct hwa_info			*hwa, *hwahead;
	struct sockaddr_un 		su;
	struct sockaddr			*sa;
	socklen_t				addrlen;
	int 					i, j;
	int 					protocol, maxfdp1;
	fd_set					rset;
	//char					address[16];
	char					rcvline[MAXLINE];
	struct hostent			*host;
	struct sockaddr		temp;
	struct  sockaddr_un	sckadr;
	
	if(argc < 2)
	{
		printf("Usage: odr staleness\n");
		return -1;
	}
	
	staleness  = atoi(argv[1]);
	unsent_init();
	rtable_init();
	dtable_init();
	
	gethostname(name, 16);
	host = malloc(sizeof(struct hostent));
	host = gethostbyname(name);
	if(host == NULL)
		printf("host is NULL\n");
	strncpy(canonical, inet_ntoa( *( struct in_addr*)( host -> h_addr_list[0])), 16);
	printf("Local ODR on host %s\n", name);
	printf( "Canonical IP: %s\n", canonical);

	printf("Finding interfaces\n");
	for (i = 0, hwahead = hwa = Get_hw_addrs(); hwa != NULL && i < MAX_INTERFACES; hwa = hwa->hwa_next, i++)
	{
		if(strncmp(hwa->if_name, "eth0", 4) != 0 && strncmp(hwa->if_name, "lo", 2) != 0)
		{
			/*
			for(j = 0; j < 6; j++)
			{
				interfaces[i].if_haddr[j] = hwa->if_haddr[j];
			}*/
			memcpy((void*)interfaces[i].if_haddr, (void*)hwa->if_haddr, 6);
			interfaces[i].if_index = hwa->if_index;
			sa = hwa->ip_addr;
			strncpy(interfaces[i].ip_addr, sock_ntop_host(sa, sizeof(*sa)), 16);
			printf("IP Address: %s\n",  interfaces[i].ip_addr);
			printf("Hardware Address: ");
			printHW(interfaces[i].if_haddr);
			printf("\nIndex: %d\n\n", interfaces[i].if_index);
		}
		else
			i--;
	}
	
	if_nums = i;
	err_msg("%s",ODR_SUNPATH);

	printf("Found %d interfaces\n", if_nums);
	//Creating packet socket
	pfsock = socket(AF_PACKET, SOCK_RAW, htons(ODR_PROTOCOL));
	if(pfsock < 0)
	{
		printf("pfsock: %d %s\n", errno, strerror(errno));
		return -1;
	}
	
	unlink(ODR_SUNPATH);
	bzero(&su, sizeof(su));
	su.sun_family = AF_LOCAL;
	strcpy(su.sun_path, ODR_SUNPATH);
	//strcat(su.sun_path, "\0");
	
	appsock = socket(AF_LOCAL, SOCK_DGRAM, 0);
	if(appsock < 0)
	{
		printf("appsock: %d %s\n", errno, strerror(errno));
		return -1;
	}
	

	if(bind(appsock, (struct sockaddr*) &su, sizeof(su)) < 0)
	{
		printf("bind failed: %d %s\n", errno, strerror(errno));
		return -1;
	}

	FD_ZERO(&rset);
	printf("Bound appsock\n");
	for ( ; ; ) 
	{

		FD_SET(pfsock, &rset);
		FD_SET(appsock, &rset);
		maxfdp1 = max(pfsock, appsock) + 1;
		
		printf("Selecting...\n");
		if(select(maxfdp1, &rset, NULL, NULL, NULL) < 0)
		{
			printf("select error: %d %s\n", errno, strerror(errno));
		}
		
		if(FD_ISSET(appsock, &rset))
		{
			printf("appsock is ready\n");
			//msg_recv(appsock, message, address, port);
			addrlen = sizeof(sckadr);
			
			if(recvfrom(appsock, rcvline, MAXLINE, 0, &temp, &addrlen) < 0)
			{
					printf("appsock recvfrom error: %d %s\n", errno, strerror(errno));
					return -1;
			}
			
			//su = (struct sockaddr_un*) sa;			
			//err_msg("%s",temp.sa_data);
			//err_msg("%d",addrlen);
			printf("Recv %s\n", rcvline);
			processAPPmsg(rcvline, &temp);
		}
		if(FD_ISSET(pfsock, &rset))
		{
			printf("pfsock is ready\n");
			
			if(recvfrom(pfsock, rcvline, MAXLINE, 0, sa, &addrlen) < 0)
			{
					printf("pfsock recvmsg error: %d %s\n", errno, strerror(errno));
					return -1;
			}
			
			printf("Received message: \n");
			printf("From: ");
			printHW(rcvline);
			
			if(!toss(rcvline))
			{
				msg = rcvline + 14;

				memcpy((void*)neighbor, (void*)rcvline + ETH_ALEN, ETH_ALEN);
				printf(" To: ");
				printHW(neighbor);
				printf("\n");
				processODRmsg(msg, sa);
			}
			else
				printf("Toss it!\n");
		}
	}
}

int toss(char *rcvline)
{
	int i = 0;
	struct ODRmsg *msg;
	
	for(i = 0; i < if_nums; i++)
	{
		if(memcmp((void*) (rcvline + ETH_ALEN), (void*)interfaces[i].if_haddr, ETH_ALEN) == 0)
		{
			printf("Broadcast loopback, toss it!\n");
			return 1;
		}
	}	
	
	msg = rcvline + 14;
	
	if(strncmp(msg->src_ip, canonical, 16) == 0)
	{
		printf("We sent this, toss it\n");
		return 1;
	}
	
	return 0;
};


int processAPPmsg(char *rcvline, struct sockaddr *sa)
{	
	printf("Processing APPmsg\n");
	//char temp[30], message[30], address[16];
	char *temp, *message, *address;
	socklen_t* address_len;
	int gr, s, port, destport, srcport, flag;
	struct ODRmsg appmsg, msg;
	struct demux *found;
	//struct sockaddr_un *su;
	
	printf("Received message from peer process\n");
	printf("Message: %s\n", rcvline);
	
	address = malloc(16);
	message = malloc(30);
	temp = malloc(50);

	err_msg("#############");
	printf("Client UNIX PATH %s\n", sa->sa_data);
	//printf("Parse %s\n", sa->sun_family);
		err_msg("#############");
	temp = strtok(sa->sa_data, "_");
	printf("Standard Prefix %s\n", temp);
	temp = strtok(NULL, "_");
	srcport=atoi(temp);
	printf(" port %d\n", srcport);
	//Get the port number from the sun_path
	
	printf("Demux Table: %d %s\n", srcport, sa->sa_data);
	found = inDemuxTable(srcport);
	if(found != NULL)
		updatetime(found); //update the entry
	else
		addNewDemux(srcport, sa->sa_data); //make a new entry
	
	//Read string
	
	strncpy(address, strtok(rcvline, " "), 16);
	printf("address %s\n", address);
	strcpy(temp, strtok(NULL, " "));
	printf("temp %s\n", temp);
	sscanf(temp, "%d", &destport);
	printf("port %d\n", destport);
	strcpy(message, strtok(NULL, " "));
	strcat(message, "\0");
	printf("message %s\n", message);
	strcpy(temp, strtok(NULL, " "));
	printf("temp %s\n", temp);
	sscanf(temp, "%d", &flag);
	printf("flag %d\n", flag);
	
	/*
	address = strtok(rcvline, " ");
	temp = strtok(NULL, " ");
	sscanf(temp, "%d", port);
	message = strtok(NULL, " ");
	temp  = strtok(NULL, " ");
	sscanf(temp, "%d", flag);*/
	
	printf("Received msg_send request from local application:\n");
	printf("Destination address: %s\n", address);
	printf("Destination port: %d\n", destport);
	printf("Destination message: %s\n", message);
	printf("Flag: %d\n", flag);
	
	strncpy(appmsg.src_ip, canonical, 16);
	strncpy(appmsg.dest_ip, address, 16);
	appmsg.app.destport = htons(destport);
	appmsg.app.srcport = htons(srcport);
	strncpy(appmsg.app.message, message, strlen(message));
	appmsg.app.msgsz = htons(strlen(message));
	appmsg.forced_discovery = htons(flag);
	
	strncpy(msg.src_ip, canonical, 16);
	strncpy(msg.dest_ip, address, 16);
	msg.hopcount = htons(0);
	
	if(flag == 1) //Force Route Discovery
	{
		sendRREQ(msg, -1, 1, 0); //struct ODRmsg oldmsg, int incoming_if, int force, int RREPsent
		APPmsgBackup(msg);
		/*
		waitforRREP(msg.dest_ip);
		gr = gotFreshRoute(msg.dest_ip);
		if(gr != -1)
			sendAPPmsg(&appmsg, gr);
		gr = gotFreshRoute(msg.dest_ip);
		sendAPPmsg(&appmsg, gr);*/
	}
	else
	{
		printAPPmsg(appmsg);
		gr = gotFreshRoute(msg.dest_ip, staleness);
		if(gr == -1)
		{
			sendRREQ(msg, -1, 0, 0);
			APPmsgBackup(appmsg);
			/*
			waitforRREP(msg.dest_ip);
			gr = gotFreshRoute(msg.dest_ip);
			if(gr != -1)
				sendAPPmsg(&appmsg, gr);
			else
				printf("waitforRREP failed and we're quitting!\n");*/
		}
		else
			sendAPPmsg(appmsg, gr);
	}
	
	//sendAPPmsg(&app);
	free(address);
	free(message);
	free(temp);

	return 0;
}

int processODRmsg(struct ODRmsg *m, struct sockaddr *sa)
{
	int gr = 0, s = 0, type;
	char temp[16];
	struct demux *found;
	struct sockaddr_ll *sall;
	struct ODRmsg msg, RREP, RREQ;

	sall = (struct sockaddr_ll *) sa;
	
	msg = *m;
	type = ntohs(msg.type);
	
	if(type == 0)
	{
		printf("Received RREQ\n");
		
		//Update the routing table with information about the neighbor
		updateTable(msg.src_ip, neighbor, sall->sll_ifindex, ntohs(msg.hopcount), ntohs(msg.forced_discovery));
		
		//If we're the destination, send an RREP
		if(isDestination(msg.dest_ip))
		{
			//Send an RREP to your neighbor
			if(ntohs(msg.RREPsent) == 0)
			{
				strncpy(RREP.src_ip, canonical, 16);
				strncpy(RREP.dest_ip, msg.src_ip, 16);
				RREP.hopcount = msg.hopcount;
				RREP.forced_discovery = msg.forced_discovery;
				sendRREP(RREP, sall->sll_ifindex, -1, msg.forced_discovery);
			}
			else
				printf("RREP already sent, do nothing\n");
		}
		else
		{
			//Check for a route in the table
			gr = gotFreshRoute(msg.dest_ip, staleness);
			if(gr == -1) //If there's no route in the table, send an RREQ
				sendRREQ(msg, sall->sll_ifindex, msg.forced_discovery, 0);
			else //If there's a fresh route, send an RREP
			{
				if(ntohs(msg.RREPsent) == 0)
				{
					strncpy(RREP.src_ip, msg.dest_ip, 16);
					strncpy(RREP.dest_ip, msg.src_ip, 16);
					RREP.hopcount = routing_table[gr].hops;
					RREP.forced_discovery = msg.forced_discovery;
					sendRREP(RREP, sall->sll_ifindex, -1, msg.forced_discovery);
				}
				else
					printf("RREP already sent, do nothing\n");
			}
		}
	}
	else if(type == 1)
	{
		printf("Received RREP\n");
		
		//Update the table
		updateTable(msg.src_ip, neighbor, sall->sll_ifindex, ntohs(msg.hopcount), ntohs(msg.forced_discovery)); //src_ip or dest_ip?
		
		if(!isDestination(msg.dest_ip))
		{
			//If you have a route, forward the RREP
			gr = gotFreshRoute(msg.dest_ip, staleness);
			if(gr != -1)
				sendRREP(RREP, routing_table[gr].index, gr, msg.forced_discovery);
			else
				printf("No route?\n");
		}
		
		sendUnsentMsgs();
	}
	else if(type == 2)
	{
		printf("Received application payload\n");
		printAPPmsg(msg);
		//Update the table
		updateTable(msg.src_ip, neighbor, sall->sll_ifindex, ntohs(msg.hopcount), ntohs(msg.forced_discovery)); //src_ip or dest_ip?
		
		//If this is the destination of the app payload, send it to the peer process
		if(isDestination(msg.dest_ip))
		{
			printf("Forwarding to peer process\n");
			sendtoDest(msg);
		}
		else
		{
			//Find a route
			gr = gotFreshRoute(msg.dest_ip, staleness);
			if(gr != -1)
			{
				//If there's no route, send an RREQ and stick the message in the UNSENT buffer
				strncpy(RREQ.src_ip, canonical, 16);
				strncpy(RREQ.dest_ip, msg.dest_ip, 16);
				RREQ.hopcount = htons(0);
				sendRREQ(RREQ, sall->sll_ifindex, -1, 1);
				APPmsgBackup(msg);
				/*
				if(waitforRREP(msg.dest_ip) == 0)
					sendAPPmsg(&msg, gr);
				else
					printf("waitforRREP failed\n");*/
			}
			else
				sendAPPmsg(msg, gr);
		}
	}
	else
		printf("Message has invalid type: %d\n", type);
	
	return 0;
}

/*
int waitforRREP(char *dest)
{
	printf("Waiting for RREPs\n");

	int i;
	struct ODRmsg *msg;
	struct sockaddr *sa;
	struct sockaddr_ll *sall;
	socklen_t addrlen;	
	char nbor[6], rcvline[MAXLINE];


	for( ; ;)
	{
		if(recvfrom(pfsock, rcvline, MAXLINE, 0, sa, &addrlen) < 0)
		{
				printf("pfsock recvmsg error: %d %s\n", errno, strerror(errno));
				return -1;
		}
		
		printf("Received message: \n");
		printHW(rcvline);
		printf("\n");
		printHW(rcvline + ETH_ALEN);

		msg = rcvline + 14;

		//If you get an RREP for your destination
		if((msg->type == ntohs(1)) && (strncmp(msg->src_ip, dest, 16) == 0))
		{
			for(i = 0; i < 6; i++)
			{
				nbor[i] = rcvline[i];
			}
			
			sall = (struct sockaddr_ll*) sa;
			updateTable(msg->src_ip , nbor, sall->sll_ifindex, ntohs(msg->hopcount), ntohs(msg->forced_discovery));
		}
	}
	return 0;
}
*/

int sendAPPmsg(struct ODRmsg app, int route)
{
	//Get the index from the routing table, use it to find the right interface;
	//printf("??? %s %s\n", app.src_ip, app.dest_ip);
	app.type = htons(2);
	app.hopcount++;
	printAPPmsg(app);
	int inter = findInterface(routing_table[route].index);
	if(inter == -1)
	{
		printf("Can't send APPmsg, invalid interface\n");
		return -1;
	}	
	
	if(sendPacket(app, inter, route, 0) == 0)
		return 0;
	else
	{
		printf("sendAPPmsg error\n");
		return -1;
	}
}

int sendRREQ(struct ODRmsg RREQ, int incoming_if, int force, int RREPsent)
{
	printf("sending RREQ for %s\n", RREQ.dest_ip);
	int i, ifi;
	
	RREQ.type = htons(0);
	RREQ.broadcast_id = htons(broadcast_id++);
	RREQ.hopcount++;
	RREQ.RREPsent = htons(RREPsent);
	RREQ.forced_discovery = htons(force);
	strncpy(RREQ.app.message, "RREQ\0", 5);
	printf("%s\n", RREQ.app.message);
	
	ifi = findInterface(incoming_if);
	
	for(i = 0; i < if_nums; i++)
	{
		if(i != ifi)
		{
			if(sendPacket(RREQ, i, -1, 1) < 0)
				printf("Failed to sendRREQ on interface %i\n", i);
		}
	}
	return 0;
}

int sendRREP(struct ODRmsg RREP, int out_if, int route, int force)
{
	printf("sending RREP\n");
	int i, ifi;
	
	RREP.type = htons(1);
	RREP.broadcast_id = htons(broadcast_id);
	RREP.forced_discovery = force; //This should already be htons
	strncpy(RREP.app.message, "RREP\0", 5);
	
	ifi = findInterface(out_if);
	if(ifi == -1)
	{
		printf("Can't send RREP, invalid interface\n");
		return -1;
	}
	
	if(sendPacket(RREP, ifi, route, 0) < 0)
	{
			printf("Failed to sendRREP on interface %i\n", out_if);
			return -1;
	}
	return 0;
}

int sendPacket(struct ODRmsg msg, int out_if, int route, int broadcast)
{
	int i;
	struct hostent *h1, *h2;
	struct sockaddr_ll socket_address;
	void* buffer = (void*)malloc(ETH_FRAME_LEN);
	unsigned char* etherhead = buffer;
	struct ODRmsg *data = buffer + 14;
	struct ethhdr *eh = (struct ethhdr *)etherhead; 

	unsigned char src_mac[6], dest_mac[6];
	
	for(i = 0; i < 6; i++)
	{
		src_mac[i] = interfaces[out_if].if_haddr[i];
		if(broadcast == 1)
		{
			dest_mac[i] = 0xFF;
			socket_address.sll_addr[i] = 0xFF;
		}
		else if(route != -1)
		{
			dest_mac[i] = routing_table[route].nexthop[i];
			socket_address.sll_addr[i] = routing_table[route].nexthop[i];
		}
		else if(route == -1)
		{
			dest_mac[i] = neighbor[i];
			socket_address.sll_addr[i] = neighbor[i];
		}
	}

	socket_address.sll_addr[6]  = 0x00;/*not used*/
	socket_address.sll_addr[7]  = 0x00;/*not used*/
	
	socket_address.sll_family   = PF_PACKET;	
	socket_address.sll_protocol = htons(ODR_PROTOCOL);	
	socket_address.sll_ifindex  = interfaces[out_if].if_index;

	socket_address.sll_hatype   = 1;
	
	if(broadcast == 1)
		socket_address.sll_pkttype  = PACKET_BROADCAST;
	else
		socket_address.sll_pkttype  = PACKET_OTHERHOST;

	socket_address.sll_halen  =  ETH_ALEN;		

	memcpy((void*)buffer, (void*)dest_mac, ETH_ALEN);
	memcpy((void*)(buffer+ETH_ALEN), (void*)src_mac, ETH_ALEN);
	memcpy((void*)data, (void*)&msg, sizeof(struct ODRmsg));
	eh->h_proto = htons(ODR_PROTOCOL);
	
	
	/*send the packet*/
	printf("Sending packet on interface index %d\n", interfaces[out_if].if_index);
	printHW(src_mac);
	printf(" to ");
	printHW(dest_mac);
	if(sendto(pfsock, buffer, ETH_FRAME_LEN, 0, (struct sockaddr*)&socket_address, sizeof(socket_address)) < 0)
	{
		printf("sendPacket sendto error: %d %s\n", errno, strerror(errno));
		return -1;
	}
	else
	{
		/*
		h1 = gethostbyaddr(msg.src_ip, sizeof(msg.src_ip), AF_INET);
		h2 = gethostbyaddr(msg.dest_ip, sizeof(msg.dest_ip), AF_INET);
		if(h1 == NULL )
			printf("h1 == NULL\n");
		if(h2 == NULL)
			printf("h2 == NULL\n");*/
		
		printf("\nODR at node %s: sending frame hdr - src: %s  dest: ", name, name);
		printHW(buffer);
		printf("\nODR msg payload - message: %s ", msg.app.message);
		printf("type: %d\n", htons(msg.type));
		//printf("src: %s  ", h1->h_name);
		//printf("dest: %s\n", h2->h_name);
		
		return 0;
	}
}

int sendtoDest(struct ODRmsg msg) //FOR APPMSG ONLY!
{
	struct demux *dest;
	struct sockaddr_un su;
	char sendline[100], port[7];
	
	printf("Port: %d\n", msg.app.destport);
	//Find the sun_path from the port number
	//dest = inDemuxTable(ntohs(msg.app.destport));
	dest = inDemuxTable(ntohs(msg.app.destport));
	if(dest != NULL)
	{
		su.sun_family = AF_LOCAL;
		strncpy(su.sun_path, dest->sun_path, strlen(dest->sun_path));
		printf("Destination sun_path: %s\n", dest->sun_path);
		
		strncpy(sendline, msg.app.message, ntohs(msg.app.msgsz));
		strcat(sendline, " ");
		sprintf(port, "%d", ntohs(msg.app.destport));
		strcat(sendline, port);
		strcat(sendline, " ");
		strncat(sendline, msg.src_ip, 16);
		strcat(sendline, "\0");
		
		printf("Sending message: %s\n", sendline);
		if(sendto(appsock, (void*) sendline, sizeof(sendline), 0, (struct sockaddr*) &su, sizeof(struct sockaddr)) < 0)
		{
			printf("sendtoDest sendto error: %d %s\n", errno, strerror(errno));
			return -1;
		}
	}
	else
	{
		printf("This destination port is not valid, sendtoDest failed\n");
		return -1;
	}
}

int sendUnsentMsgs()
{
	printf("Trying to send unsent messages...\n");
	int i, gr;
	for(i = 0; i < MAX_UNSENT; i++)
	{
		if(unsent[i].empty != 0)
		{
			printf("Resending to from %s to %s\n", unsent[i].msg.src_ip, unsent[i].msg.dest_ip);
			gr = gotFreshRoute(unsent[i].msg.dest_ip, staleness); //Check to see
			if(gr != -1)
			{
				sendAPPmsg(unsent[i].msg, gr);
			}
			unsent_num--;
			unsent[i].empty = 0; //Empty the slot
		}
	}

	return 0;
}

int APPmsgBackup(struct ODRmsg app)
{
	int i;
	for(i = 0; i < MAX_UNSENT; i++)
	{
		if(unsent[i].empty == 0)
		{
			unsent[i].msg = app;
			printf("Unsent message for %s\n", unsent[i].msg.dest_ip);
			unsent[i].empty = 1;
			unsent_num++;
			printf("Placing message in UNSENT queue, %d unsent message(s)\n", unsent_num);
			return 0;
		}
	}
	
	printf("UNSENT buffer full\n");
	return -1;
}

void unsent_init()
{
	int i;
	for(i = 0; i < MAX_UNSENT; i++)
	{
		unsent[i].empty = 0;
	}
}

int isDestination(char *dest)
{
	printf("Destination: %s\n", dest);
	printf("Current Location: %s\n", canonical);
	if(strncmp(dest, canonical, 16) == 0)
	{
		printf("Destination Reached!\n");
		return 1;
	}
	
	printf("This is not the destination!\n");
	return 0;
}

int findInterface(int if_index)
{
	int i;
	for(i = 0; i < if_nums; i++)
	{
		if(interfaces[i].if_index == if_index)
			return i;
	}
	
	return -1;
}

void printHW(char* ptr)
{
	int i = 6;
	do 
	{
		printf("%.2x%s", *ptr++ & 0xff, (i == 1) ? " " : ":");
	} while (--i > 0);
}


void printAPPmsg(struct ODRmsg msg)
{
	printf("Printing APP msg: \n");
	printf("Type: %d\n", msg.type);
	//printf("SrcIP: %s\n", msg.src_ip[16]);
	//printf("DestIP: %s\n", msg.dest_ip[16]);
	printf("SrcPort: %d\n", msg.app.srcport);
	printf("DestPort: %d\n", msg.app.destport);
	printf("Msgsz: %d\n", msg.app.msgsz);
	printf("Message: %s\n", msg.app.message);
	printf("Hop Count: %d\n", msg.hopcount);
	printf("Force: %d\n", msg.forced_discovery);
}
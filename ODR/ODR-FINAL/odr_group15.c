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

int main(int argc, char **argv)
{
	struct ODRmsg			*msg;
	struct hwa_info			*hwa, *hwahead;
	socklen_t				addrlen;
	struct sockaddr_un 		su;
	struct sockaddr		temp;
	struct  sockaddr_un	sckadr;
	struct sockaddr *sa;
	int 					i, j;
	int 					protocol, maxfdp1;
	fd_set					rset;
	char					rcvline[MAXLINE];
	struct hostent			*host;
	time_t					last_time;
	
	//If a staleness parameter is not included, use a default value of 10 seconds
	if(argc < 2)
	{
		err_msg("No Staleness value provided, Using the default value 10");
		staleness=10;
	}
	else
	{
		staleness  = atoi(argv[1]);
	}
	
	//Initialize tables
	unsent_init();
	rtable_init();
	dtable_init();

	//Get host name and canonical IP address
	gethostname(name, 16);
	host = malloc(sizeof(struct hostent));
	host = gethostbyname(name);
	if(host == NULL)
		printf("host is NULL\n");
	strncpy(canonical, inet_ntoa( *( struct in_addr*)( host -> h_addr_list[0])), 16);
	printf("Local ODR on host %s\n", name);
	printf( "Canonical IP: %s\n", canonical);

	printf("Finding interfaces\n");
	//Save information about the interfaces (MAC address, index, and IP address) for future use)
	for (i = 0, hwahead = hwa = Get_hw_addrs(); hwa != NULL && i < MAX_INTERFACES; hwa = hwa->hwa_next, i++)
	{
		if(strncmp(hwa->if_name, "eth0", 4) != 0 && strncmp(hwa->if_name, "lo", 2) != 0)
		{
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
	printf("Found %d interfaces\n", if_nums);
	
	//Create packet socket
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

	//Create domain socket
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

	//Enter main loop
	FD_ZERO(&rset);
	for ( ; ; ) 
	{
		//select() set up
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
			addrlen = sizeof(sckadr);
			
			if(recvfrom(appsock, rcvline, MAXLINE, 0, &temp, &addrlen) < 0)
			{
					printf("appsock recvfrom error: %d %s\n", errno, strerror(errno));
					return -1;
			}

			processAPPmsg(rcvline, &temp);
		}
		if(FD_ISSET(pfsock, &rset))
		{
			if(recvfrom(pfsock, rcvline, MAXLINE, 0, sa, &addrlen) < 0)
			{
					printf("pfsock recvmsg error: %d %s\n", errno, strerror(errno));
					return -1;
			}
			
			memcpy((void*)neighbor, (void*)rcvline + ETH_ALEN, ETH_ALEN);
			
			//Toss the message if we sent it
			if(!toss(neighbor))
			{
				msg = rcvline + 14;
				processODRmsg(msg, sa);
			}
			else
				printf("Toss it!\n");
				
			//Every so often, clean up the demultiplexing table
			if(difftime(last_time, time(NULL)) > staleness)
			{
				purge();
				last_time = time(NULL);
			}
		}
	}
}

int toss(char *rcvline)
{
	int i = 0;
	struct ODRmsg *msg;
	
	//if the message came from us and has been looped back, throw it away
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
	char *temp, *message, *address;
	socklen_t* address_len;
	int gr, s, port, destport, srcport, flag;
	struct ODRmsg appmsg, msg;
	struct demux *found;
	
	printf("Received message from peer process\n");
	printf("Message: %s\n", rcvline);
	
	address = malloc(16);
	message = malloc(30);
	temp = malloc(50);
	
	//get the port number from the sun path
	char temp2[20];
	strncpy(temp2, sa->sa_data, 20);
	temp = strtok(temp2, "_");
	temp = strtok(NULL, "_");
	srcport=atoi(temp);
	printf(" port %d\n", srcport);

	//put the port number in the demultiplexing table
	if(srcport != 0)
	{
		printf("Demux Table: %d %s\n", srcport, sa->sa_data);
		found = inDemuxTable(srcport);
		if(found != NULL)
			updatetime(found); //update the entry
		else
			addNewDemux(srcport, sa->sa_data); //make a new entry*/
	}
	else
		printf("Peer process is a server\n");
	
	//parse string	
	strncpy(address, strtok(rcvline, "-"), 16);
	strcpy(temp, strtok(NULL, "-"));
	sscanf(temp, "%d", &destport);
	strcpy(message, strtok(NULL, "-"));
	message[strlen(message)] = '\0';
	strcpy(temp, strtok(NULL, "-"));
	sscanf(temp, "%d", &flag);
	
	printf("Received msg_send request from local application:\n");
	printf("Destination address: %s\n", address);
	printf("Destination port: %d\n", destport);
	printf("Destination message: %s\n", message);
	printf("Flag: %d\n", flag);
	
	//create application payload message
	strncpy(appmsg.src_ip, canonical, 16);
	strncpy(appmsg.dest_ip, address, 16);
	appmsg.app.destport = htons(destport);
	appmsg.app.srcport = htons(srcport);
	appmsg.hopcount = htons(0);
	strncpy(appmsg.app.message, message, strlen(message));
	appmsg.app.msgsz = htons(strlen(message));
	appmsg.forced_discovery = htons(flag);
	
	//create RREQ
	strncpy(msg.src_ip, canonical, 16);
	strncpy(msg.dest_ip, address, 16);
	msg.hopcount = htons(0);
	
	if(flag == 1) //Force Route Discovery
	{
		printf("Forcing route discovery\n");
		sendRREQ(msg, -1, htons(1), htons(0)); //send an RREQ
		APPmsgBackup(msg); //place the app payload in the unsent buffer
	}
	else
	{
		gr = gotFreshRoute(msg.dest_ip, staleness); //find a route
		if(gr == -1)
		{
			sendRREQ(msg, -1, htons(0), htons(0)); //send an RREQ
			APPmsgBackup(appmsg); //place the app payload in the unsent buffer
		}
		else
			sendAPPmsg(appmsg, gr); //send message
	}

	return 0;
}

int processODRmsg(struct ODRmsg *m, struct sockaddr *sa)
{
	int gr = 0, s = 0, type, RREPsend = 0;
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
		
		//continue to flood the RREQ it received if the RREQ is from a
		//source node we were unaware of, or the RREQ gives it a more efficient route than it knew of back to the source node
		gr = findInTable(msg.src_ip);
		if(gr == -1)
		{
			printf("Setting RREPsend\n");
			RREPsend = 1;
		}
		else
		{
			gr = gotFreshRoute(msg.src_ip, staleness);
			if(gr != -1 && routing_table[gr].hops > ntohs(msg.hopcount))
			{
				printf("Setting RREPsend\n");
				RREPsend = 1;
			}
		}
		
		//Update the routing table with information about the neighbor
		printf("Updating table from RREQ: Destination: %s, Index: %d, Hopcount: %d, Force :%d\n", msg.src_ip, sall->sll_ifindex, ntohs(msg.hopcount), ntohs(msg.forced_discovery));
		updateTable(msg.src_ip, neighbor, sall->sll_ifindex, ntohs(msg.hopcount), ntohs(msg.forced_discovery));
		
		//If we're the destination, send an RREP
		if(isDestination(msg.dest_ip))
		{
			//Send an RREP to your neighbor
			if(ntohs(msg.RREPsent) == 0)
			{
				strncpy(RREP.src_ip, canonical, 16);
				strncpy(RREP.dest_ip, msg.src_ip, 16);
				RREP.hopcount = ntohs(msg.hopcount) + 1;
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
			if(gr == -1 || (ntohs(msg.forced_discovery) == 1))
				sendRREQ(msg, sall->sll_ifindex, msg.forced_discovery, htons(0));
			else //If there's a fresh route, send an RREP
			{
				if(ntohs(msg.RREPsent) == 0)
				{						
					strncpy(RREP.src_ip, msg.dest_ip, 16);
					strncpy(RREP.dest_ip, msg.src_ip, 16);
					RREP.hopcount = htons(routing_table[gr].hops + 1);
					RREP.forced_discovery = msg.forced_discovery;
					sendRREP(RREP, sall->sll_ifindex, -1, msg.forced_discovery);
					
					if(RREPsend == 1)
						sendRREQ(msg, sall->sll_ifindex, msg.forced_discovery, htons(1));
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
		printf("Updating table from RREP: Destination: %s, Index: %d, Hopcount: %d, Force :%d\n", msg.src_ip, sall->sll_ifindex, ntohs(msg.hopcount), ntohs(msg.forced_discovery));
		updateTable(msg.src_ip, neighbor, sall->sll_ifindex, ntohs(msg.hopcount), ntohs(msg.forced_discovery));
		
		if(!isDestination(msg.dest_ip))
		{
			//If you have a route, forward the RREP
			gr = gotFreshRoute(msg.dest_ip, staleness);
			if(gr != -1)
			{
				strncpy(RREP.src_ip, msg.src_ip, 16);
				strncpy(RREP.dest_ip, msg.dest_ip, 16);
				RREP.hopcount = htons(routing_table[gr].hops + 1);
				RREP.forced_discovery = msg.forced_discovery;
				sendRREP(RREP, routing_table[gr].index, gr, msg.forced_discovery);
			}
			else
				printf("No route?\n");
		}
		
		//Attempt to send unsent messages
		sendUnsentMsgs();
	}
	else if(type == 2)
	{
		printf("Received application payload\n");

		//Update the table
		printf("Updating table from app payload: Destination: %s, Index: %d, Hopcount: %d, Force :%d\n", msg.src_ip, sall->sll_ifindex, ntohs(msg.hopcount), ntohs(msg.forced_discovery));
		updateTable(msg.src_ip, neighbor, sall->sll_ifindex, ntohs(msg.hopcount), ntohs(msg.forced_discovery));
		
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
			if(gr == -1)
			{
				//If there's no route, send an RREQ and stick the message in the UNSENT buffer
				strncpy(RREQ.src_ip, canonical, 16);
				strncpy(RREQ.dest_ip, msg.dest_ip, 16);
				RREQ.hopcount = htons(0);
				sendRREQ(RREQ, sall->sll_ifindex, msg.forced_discovery, htons(0));
				APPmsgBackup(msg);
			}
			else
				sendAPPmsg(msg, gr);
		}
	}
	else
		printf("Message has invalid type: %d\n", type);
	
	return 0;
}

int sendAPPmsg(struct ODRmsg app, int route)
{
	int newhop = ntohs(app.hopcount) + 1;

	//Get the index from the routing table, use it to find the right interface;
	app.type = htons(2);
	app.hopcount = htons(newhop);
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
	int i, ifi, newhop;
	
	newhop = ntohs(RREQ.hopcount) + 1;
	RREQ.type = htons(0);
	RREQ.broadcast_id = htons(broadcast_id++);
	RREQ.hopcount = htons(newhop);
	RREQ.RREPsent = RREPsent;
	RREQ.forced_discovery = force;
	strncpy(RREQ.app.message, "RREQ\0", 5);
	
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
	int i, ifi;
	
	RREP.type = htons(1);
	RREP.broadcast_id = htons(broadcast_id);
	RREP.forced_discovery = force;
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
	struct hostent *h;
	struct sockaddr_in tempaddr;
	struct in_addr **pptr;
	struct sockaddr_ll socket_address;
	void* buffer = (void*)malloc(ETH_FRAME_LEN);
	unsigned char* etherhead = buffer;
	struct ODRmsg *data = buffer + 14;
	struct ethhdr *eh = (struct ethhdr *)etherhead; 
	char hostName[10];

	unsigned char src_mac[6], dest_mac[6];
	
	//Set the source and destination MAC address
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
	
	//set socket_address values
	socket_address.sll_family   = PF_PACKET;	
	socket_address.sll_protocol = htons(ODR_PROTOCOL);	
	socket_address.sll_ifindex  = interfaces[out_if].if_index;

	socket_address.sll_hatype   = 1;
	
	if(broadcast == 1)
		socket_address.sll_pkttype  = PACKET_BROADCAST;
	else
		socket_address.sll_pkttype  = PACKET_OTHERHOST;

	socket_address.sll_halen  =  ETH_ALEN;		

	//set up header
	memcpy((void*)buffer, (void*)dest_mac, ETH_ALEN);
	memcpy((void*)(buffer+ETH_ALEN), (void*)src_mac, ETH_ALEN);
	memcpy((void*)data, (void*)&msg, sizeof(struct ODRmsg));
	eh->h_proto = htons(ODR_PROTOCOL);
	
	
	//send the packet
	if(sendto(pfsock, buffer, ETH_FRAME_LEN, 0, (struct sockaddr*)&socket_address, sizeof(socket_address)) < 0)
	{
		printf("sendPacket sendto error: %d %s\n", errno, strerror(errno));
		return -1;
	}
	else
	{
		//print out information about the sent packet
		bzero(&tempaddr, sizeof(tempaddr));
		tempaddr.sin_family = AF_INET;

		if (inet_pton(AF_INET, msg.src_ip, &tempaddr.sin_addr) <= 0){ /* Get IP address and name for the source node */
		err_msg("inet_pton error for %s", msg.src_ip);
		}
		pptr = &tempaddr.sin_addr;
		h = gethostbyaddr(pptr, sizeof(pptr), AF_INET);

		strcpy(hostName, h->h_name);

		if (inet_pton(AF_INET, msg.dest_ip, &tempaddr.sin_addr) <= 0){ /* get IP address and name for the destination node */
		err_msg("inet_pton error for %s", msg.dest_ip);
		}

		pptr = &tempaddr.sin_addr;
		h = gethostbyaddr(pptr, sizeof(pptr), AF_INET);
		printf("\nODR at node %s: sending frame hdr - src: %s,  dest: ", name, name);
		printHW(buffer);
		printf("\nODR msg payload - message: %s, ", msg.app.message);
		printf("type: %d, ", ntohs(msg.type));
		printf("src: %s, ", hostName);
		printf("dest: %s\n", h->h_name);

		return 0;
	}
}

int sendtoDest(struct ODRmsg msg)
{
	struct demux *dest;
	struct sockaddr_un su;
	char sendline[100], port[7];
	
	//Find the sun_path from the port number
	dest = inDemuxTable(ntohs(msg.app.destport));
	
	if(dest != NULL)
	{
		bzero(&su, sizeof(su));
		su.sun_family = AF_LOCAL;
		strncpy(su.sun_path, dest->sun_path, strlen(dest->sun_path));
		strcat(su.sun_path, "\0");
		
		strncpy(sendline, msg.src_ip, 16);
		strcat(sendline, "-");
		sprintf(port, "%d", ntohs(msg.app.srcport));
		strcat(sendline, port);
		strcat(sendline, "-");
		strncat(sendline, msg.app.message, ntohs(msg.app.msgsz));
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
			//Attempt to find fresh routes for any packets in the unsent buffer
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

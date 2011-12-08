#include	"unp.h"
#include "arp.h"

int processARP(struct arp_message* req, struct sockaddr* sa);
int processAREQ(char* rcvline);
void table_init();
void printHW(char* addr);
void print_ip_hw_pairs();
void printARP(struct arp_message msg);
int addEntry(char* ip_addr, char* haddr, int index, int hatype, int sockfd, int place);

int if_nums, listensock, connsock, pfsock, lastentry;
char requester[6], eth0_haddr[6];

int main(int argc, char **argv)
{
	struct sockaddr sa;
	socklen_t addrlen, clilen;
	struct sockaddr_un	servaddr, cliaddr;
	int maxfd;
	fd_set fdset;
	char rcvline[MAXLINE];
	struct arp_message* msg;
	
	//Prints the IP,HW addr pair for all aliases for interface eth0
	table_init();
	print_ip_hw_pairs();

	
	//Raw PF_PACKET socket to get the HW addr on request
	pfsock = socket(PF_PACKET, SOCK_RAW, htons(PROTO_TYPE));
	if(pfsock < 0)
	{
		printf("pfsock: %d %s\n", errno, strerror(errno));
		return -1;
	}
	

	//Domain socket for communicating with local tour
	unlink(ARP_DG_PATH);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sun_family = AF_LOCAL;
	strcpy(servaddr.sun_path, ARP_DG_PATH);
	
	listensock = socket(AF_LOCAL, SOCK_STREAM, 0);
	if(listensock < 0)
	{
		printf("listensock: %d %s\n", errno, strerror(errno));
		return -1;
	}
	if(bind(listensock, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0)
	{
		printf("bind error: %d %s\n", errno, strerror(errno));
		return -1;
	}
	
	if(listen(listensock, 5) < 0)
	{
		printf("listen error: %d %s\n", errno, strerror(errno));
		return -1;
	}

	//Infinite loop that selects between domain and raw socket
	for(;;)
	{
		//Init for socket descriptor set
		FD_ZERO(&fdset);
		FD_SET(pfsock, &fdset);
		FD_SET(listensock, &fdset);
		maxfd = max(pfsock, listensock) + 1;
		printf("Selecting...\n");
		if(select(maxfd, &fdset, NULL, NULL, NULL) < 0)
		{
			printf("select error: %d %s\n", errno, strerror(errno));
		}
		
		/*
		* Data on packet socket RAW
		*/
		if(FD_ISSET(pfsock,&fdset))
		{
			printf("pfsock ready\n");
			addrlen = sizeof(struct sockaddr);
			if(recvfrom(pfsock, rcvline, MAXLINE, 0, &sa, &addrlen) < 0)
			{
					printf("pfsock recvfrom error: %d %s\n", errno, strerror(errno));
					return -1;
			}
			
			memcpy((void*)requester, (void*)rcvline + ETH_ALEN, ETH_ALEN);
			printHW(rcvline + ETH_ALEN);
			printf(" to ");
			printHW(rcvline);
			printf("\n");
			msg = rcvline + 14;
			processARP(msg, &sa);
		}
		
		//Data on domain
		if(FD_ISSET(listensock,&fdset))
		{
			printf("listensock ready\n");
			clilen = sizeof(cliaddr);
			connsock = accept(listensock, (struct sockaddr*) &cliaddr, &clilen); 
			printf("accepting on connsock\n");
			if(connsock < 0)
			{
				printf("accept error: %d %s\n", errno, strerror(errno));
				return -1;
			}
			printf("receiving on connsock\n");
			addrlen = sizeof(struct sockaddr);
			if(recvfrom(connsock, rcvline, MAXLINE, 0, &sa, &addrlen) < 0)
			{
					printf("listensock recvfrom error: %d %s\n", errno, strerror(errno));
					return -1;
			}
			
			printf("processing AREQ: %s\n", rcvline);
			processAREQ(rcvline);
		}
	}
}

int sendRequest(char* address)
{
	int i;
	struct arp_message request;
	
	
	strncpy(request.sender_ip, arp_table[0].ip_addr, 16);
	strncpy(request.target_ip, address, 16);
	memcpy((void*)request.sender_haddr, (void*)arp_table[0].if_haddr, 6);
	memset((void*)request.target_haddr, 0, sizeof(request.target_haddr));

	request.op = htons(1);
	request.id = htons(ID_NUM);
	request.hard_type = htons(1);
	request.proto_type = htons(0x0800);
	request.hard_size = htons(6);
	request.proto_size = htons(4);
	
	printf("Sending ARP Request message\n");	
	printARP(request);
	
	for(i = 2; i < if_nums; i++)
	{
		if(sendPacket(request, i, 1) == -1)
		{
			printf("Sendpacket failed!\n");
		}
	}
	return 0;
}

int sendReply(struct arp_message request, int cache)
{
	struct arp_message reply;
	
	strncpy(reply.sender_ip, request.target_ip, 16);
	strncpy(reply.target_ip, request.sender_ip, 16);
	memcpy((void*)reply.target_haddr, (void*)request.sender_haddr, 6);
	memcpy((void*)reply.sender_haddr, (void*)arp_table[cache].if_haddr, 6);
	
	reply.op = htons(2);
	reply.id = htons(ID_NUM);
	reply.hard_type = htons(1);
	reply.proto_type = htons(0x0800);
	reply.hard_size = htons(6);
	reply.proto_size = htons(4);
	
	printf("Sending ARP Reply message\n");
	printARP(reply);
	
	if(sendPacket(reply, cache, 0) == -1)
	{
		printf("Sendpacket failed!\n");
	}
	
	return 0;
}

int sendPacket(struct arp_message msg, int index, int broadcast)
{
	int i;
	struct hostent *h;
	struct sockaddr_in tempaddr;
	struct in_addr **pptr;
	struct sockaddr_ll socket_address;
	void* buffer;
	unsigned char* etherhead; 
	struct ODRmsg *data;
	struct ethhdr *eh;
	unsigned char src_mac[6], dest_mac[6];

	buffer = (void*)malloc(ETH_FRAME_LEN);
	etherhead = buffer;
	data = buffer + 14;
	eh = (struct ethhdr *)etherhead; 
	
	//Set the source and destination MAC address
	memcpy((void*)src_mac, (void*)msg.sender_haddr, 6);
	if(broadcast == 0)
	{
		
		memcpy((void*)dest_mac, (void*)msg.target_haddr, 6);
		memcpy((void*)socket_address.sll_addr, (void*)msg.target_haddr, 6);
	}
	else
	{
		for(i = 0; i < 6; i++)
		{
			dest_mac[i] = 0xFF;
			socket_address.sll_addr[i] = 0xFF;
		}

	}
	

	socket_address.sll_addr[6]  = 0x00;/*not used*/
	socket_address.sll_addr[7]  = 0x00;/*not used*/
	
	//set socket_address values
	socket_address.sll_family   = PF_PACKET;	
	socket_address.sll_protocol = htons(PROTO_TYPE);	
	socket_address.sll_ifindex  = arp_table[index].if_index;

	socket_address.sll_hatype   = 1;
	
	if(broadcast == 1)
		socket_address.sll_pkttype  = PACKET_BROADCAST;
	else
		socket_address.sll_pkttype  = PACKET_OTHERHOST;

	socket_address.sll_halen  =  ETH_ALEN;		
	
	//set up header
	memcpy((void*)buffer, (void*)dest_mac, ETH_ALEN);
	memcpy((void*)(buffer+ETH_ALEN), (void*)src_mac, ETH_ALEN);
	memcpy((void*)data, (void*)&msg, sizeof(struct arp_message));
	eh->h_proto = htons(PROTO_TYPE);
	
	//send the packet
	if(sendto(pfsock, buffer, ETH_FRAME_LEN, 0, (struct sockaddr*)&socket_address, sizeof(socket_address)) < 0)
	{
		printf("sendPacket sendto error: %d %s\n", errno, strerror(errno));
		return -1;
	}
	else
	{
		printf("Sending message from: ");
		printHW(src_mac);
		printf(" to ");
		printHW(dest_mac);
		printf("\n");
	}
		
	return 0;
}

int checktable(char* target_ip)
{
	int i;
	
	printf("Check table for address %s\n", target_ip);
	for(i = 0; i < tablesize; i++)
	{
		if(strncmp(arp_table[i].ip_addr, target_ip, 16) == 0)
		{
			printf("Address is in the ARP table\n");
			return i;
		}
	}
	
	printf("Address not found\n");
	return -1;
}

int addEntry(char* ip_addr, char* haddr, int index, int hatype, int sockfd, int place)
{
	int i;
	
	/*
	index = ntohs(index);
	hatype = ntohs(hatype);
	sockfd = ntohs(sockfd);*/
	
	printf("Attempting to add entry: %s, ", ip_addr);
	printHW(haddr);
	printf(", %d, %d, %d\n", index, hatype, sockfd);
	
	if(place == -1)
	{
		//Entry is new, find a new slot
		if((tablesize + 1) < MAX_ARP_ITEMS)
		{
			tablesize++;
			strncpy(arp_table[tablesize].if_haddr, haddr, 6);
			arp_table[tablesize].if_index = index;
			strncpy(arp_table[tablesize].ip_addr, ip_addr, 16);
			arp_table[tablesize].sll_hatype = hatype;
			arp_table[tablesize].sockfd = sockfd;
			lastentry = tablesize;
			printf("%d items in the table\n", tablesize);
			return lastentry;
		}
		else
		{
			for(i = 0; i < MAX_ARP_ITEMS; i++)
			{
				if(arp_table[i].ip_addr[0] == '\0');
				{
					strncpy(arp_table[i].if_haddr, haddr, 6);
					arp_table[i].if_index = index;
					strncpy(arp_table[i].ip_addr, ip_addr, 16);
					arp_table[i].sll_hatype = hatype;
					arp_table[i].sockfd = sockfd;
					lastentry = i;
					return i;
				}
			}
		}

		printf("Table is full! %d\n", tablesize);
		return -1;
	}
	else
	{
		i = place;
		memcpy((void*)arp_table[i].if_haddr, (void*)haddr, 6);
		arp_table[i].if_index = index;
		strncpy(arp_table[i].ip_addr, ip_addr, 16);
		arp_table[i].sll_hatype = hatype;
		arp_table[i].sockfd = sockfd;
		lastentry = i;
		return i;
	}
	

}

int deleteEntry(int i)
{
	if(i < 0 || i >= MAX_ARP_ITEMS)
	{
		printf("Trying to delete out of bounds entry! Failed!\n");
		return -1;
	}
	
	arp_table[i].ip_addr[0] = '\0';
	arp_table[i].if_haddr[0] = '\0';	
	arp_table[i].if_index = 0;
	arp_table[i].sll_hatype = 0;
	arp_table[i].sockfd = 0;
	
	return 0;
}

int AREQresponse(int table, int halen, int waiting)
{
	fd_set fdset;
	int t, maxfd, bytes = 0;
	struct hwaddr hw;
	struct sockaddr sa;
	socklen_t addrlen;
	struct sockaddr_ll *sal;
	char rcvline[MAXLINE];
	struct arp_message* msg;
	
	//if we're waiting on an ARP reply, we need to select() on connsock and pfsock
	if(waiting)
	{
		FD_ZERO(&fdset);
		FD_SET(pfsock, &fdset);
		FD_SET(connsock, &fdset);
		maxfd = max(pfsock, connsock) + 1;
		printf("Waiting for ARP Reply...\n");
		if(select(maxfd, &fdset, NULL, NULL, NULL) < 0)
		{
			printf("select error: %d %s\n", errno, strerror(errno));
		}
		
		if(FD_ISSET(pfsock, &fdset)) //the ARP reply came in
		{
			printf("pfsock is ready\n");
			if(recvfrom(pfsock, rcvline, MAXLINE, 0, &sa, &addrlen) < 0)
			{
					printf("pfsock recvfrom error: %d %s\n", errno, strerror(errno));
					return -1;
			}
			
			msg = rcvline + 14;
			printf("ID number = %d, ID_NUM = %d\n", ntohs(msg->id), ID_NUM);
			if(ntohs(msg->id) == ID_NUM)
			{
				printf("ARP Received\n");
				sal = (struct sockaddr_ll*) &sa;
				addEntry(msg->sender_ip, msg->sender_haddr, sal->sll_ifindex, sal->sll_hatype, connsock, lastentry);
				t = lastentry;
			}
			else
				printf("ID Number is invalid\n");
		}
		if(FD_ISSET(connsock, &fdset)) //the connection was closed
		{
			bytes = recvfrom(connsock, rcvline, MAXLINE, 0, NULL, NULL);
			if(bytes == 0)
			{
				printf("areq() timeout: socket closed\n");
				deleteEntry(lastentry);
				close(connsock);
				return 0;
			}
			printf("Received %d bytes?\n", bytes);
		}
	
	}
	else
		t = table;
	
	hw.sll_ifindex = htons(arp_table[t].if_index);
	hw.sll_halen = halen;
	hw.sll_hatype = arp_table[t].sll_hatype;
	memcpy((void*)hw.sll_addr, (void*)arp_table[t].if_haddr, 6);
	hw.sll_addr[6] = 0x00;
	hw.sll_addr[7] = 0x00;
	
	printf("Sending hwaddr to api\n");
	if(send(arp_table[t].sockfd, &hw, sizeof(struct hwaddr), 0) < 0)
	{
		if(errno == EPIPE && waiting == 1) //areq timeout, socket is closed
		{
			return deleteEntry(t);
		}
		else
			printf("AREQresponse sendto fail: %d %s\n", errno, strerror(errno));
		return -1;
	}
	
	close(connsock);
	arp_table[t].sockfd = -1;
	return 0;
}

int processAREQ(char* rcvline)
{
	int table, index, hatype, halen;
	char *address, *temp;
	
	//address = malloc(16);
	
	address = strtok(rcvline, ",");
	temp = strtok(NULL, ",");
	index = atoi(temp);
	temp = strtok(NULL, ",");
	hatype = atoi(temp);
	temp = strtok(NULL, ",");
	halen = atoi(temp);
	temp[0] = '\0';
	
	printf("address: %s, index: %d, hatype: %d, halen: %d\n", address, index, hatype, halen);
	table = checktable(address);
	
	if(table != -1) //if we have it in the table
	{
		AREQresponse(table, halen, 0);
	}
	else
	{
		addEntry(address, temp, index, hatype, connsock, -1);
		sendRequest(address);
		AREQresponse(table, halen, 1);
	}
	
	return 0;
}


int processARP(struct arp_message* msg, struct sockaddr* sa)
{
	int table;
	struct sockaddr_ll *sal;
	
	//Make sure the ID number is correct
	if(ntohs(msg->id) != ID_NUM)
	{
		printf("Request has invalid ID type, %d != %d\n", ID_NUM,  ntohs(msg->id));
		return -1;
	}
	
	if(ntohs(msg->op) == 1) //received ARP request
	{
		printf("Received ARP Request\n");
		table = checktable(msg->target_ip);
		
		if(table != -1)
		{
			sendReply(*msg, table);
		}
		else
		{
			printf("ARP entry not found, do not respond to request\n");
			return 0;
		}
	}
	/*
	else if(ntohs(msg->op) == 2) //received ARP reply
	{
		printf("Received ARP Reply\n");
		printARP(*msg);
		
		//add entry to the table
		sal = (struct sockaddr_ll*) sa;
		addEntry(msg->sender_ip, msg->sender_haddr, sal->sll_ifindex, sal->sll_hatype, connsock, -1);
	}*/
}

void table_init()
{
	int i;
	for(i = 0; i < MAX_ARP_ITEMS; i++)
	{
		arp_table[i].if_haddr[0] = '\0';
		arp_table[i].if_index = 0;
		arp_table[i].ip_addr[0] = '\0';
		arp_table[i].sll_hatype;
		arp_table[i].sockfd = 0;
	}
}

void printARP(struct arp_message msg)
{
	printf("ARP Message: \n");
	printf("ID: %d\n", ntohs(msg.id));
	printf("Hard Type: %d\n", ntohs(msg.hard_type));
	printf("Proto Type: %d\n", ntohs(msg.proto_type));
	printf("Op: %d\n", ntohs(msg.op));
	printf("Hard Size: %d\n", ntohs(msg.hard_size));
	printf("Proto Size: %d\n", ntohs(msg.proto_size));
	printf("Sender IP: %s\n", msg.sender_ip);
	printf("Sender HW: ");
	printHW(msg.sender_haddr);
	printf("\n");
	printf("Target IP: %s\n", msg.target_ip);
	printf("Target HW: ");
	printHW(msg.target_haddr);
	printf("\n");
}

void printHW(char* ptr)
{
	int i = 6;
	do 
	{
		printf("%.2x%s", *ptr++ & 0xff, (i == 1) ? " " : ":");
	} while (--i > 0);
}

void print_ip_hw_pairs(){
		struct hwa_info			*hwa, *hwahead;
		struct sockaddr			*sa;
		int i, first = 0;
		struct interface		interfaces[MAX_INTERFACES];
		printf("interfaces IP Address,HW address pair \n");
		printf("_________________________________________________________\n");
		printf("IPAddress\t\tHWAddr\n");
		printf("_________________________________________________________\n");

		for (i = 0, hwahead = hwa = Get_hw_addrs(); hwa != NULL && i < MAX_INTERFACES; hwa = hwa->hwa_next, i++)
		{
			if( strncmp(hwa->if_name, "lo", 2) != 0) //Ignore the loopback
			{
				if(strncmp(hwa->if_name, "eth0", 4) == 0 && first == 0) //if it's eth0, remember the mac address for later
				{
					memcpy((void*)eth0_haddr, (void*)hwa->if_haddr, 6);
					first = 1;
				}
				
				memcpy((void*)arp_table[i].if_haddr, (void*)hwa->if_haddr, 6);
				arp_table[i].if_index = hwa->if_index;
				sa = hwa->ip_addr;
				strncpy(arp_table[i].ip_addr, sock_ntop_host(sa, sizeof(*sa)), 16);
				printf("\nIP Address: %s\t\t",  arp_table[i].ip_addr);
				printf("\nHW Address: ");
				printHW(arp_table[i].if_haddr);
				printf("\n");
				//printf("\nIndex: %d\n\n", interfaces[i].if_index);
			}
			else
				i--;
		}
		
		if_nums = i;
		tablesize = i;
		printf("Found %d interfaces\n", i);
	printf("___________________________________________________________\n");
}


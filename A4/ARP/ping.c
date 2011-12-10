#include "a.h"
#include "arp.h"
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <linux/if_ether.h>

void printHW(char* ptr);
void verify(char* buffer, int flag);

int ping_send(int sockfd, char* src_addr, char* dst_addr, char* src_mac, char* dest_mac, int index)
{
	int len, datalen = 8, sent;
	struct icmp	*icmp;
	struct iphdr *ip;
	char* packet;
	void* buffer;
	//char* data;
	struct sockaddr_ll socket_address;
	unsigned char* etherhead; 
	struct ethhdr *eh;
	struct sockaddr_in connection;
	socklen_t addrlen = sizeof(connection);
	
	//packet = malloc(sizeof(struct iphdr) + sizeof(struct icmp));
	//ip = malloc(sizeof(struct iphdr));
	//icmp = malloc(sizeof(struct icmp));
	buffer = (void*)malloc(ETH_FRAME_LEN);
	memset((void*)buffer, 0, ETH_FRAME_LEN);
	
	etherhead = buffer;
	eh = (struct ethhdr *)etherhead;

	packet = buffer + 14;
	ip = (struct iphdr*) packet;
	icmp = (struct icmp *) (packet + sizeof(struct iphdr));
	
	//data = buffer + 14;
	 	
	/* ip header */
	ip->ihl          = 5;
	ip->version      = 4;
	ip->tos          = 0;
	ip->tot_len      = sizeof(struct iphdr)  + sizeof(struct icmp);
	ip->id           = htons(ID);
	ip->ttl          = 255;
	ip->protocol     = IPPROTO_ICMP;
	ip->saddr        = inet_addr(src_addr);
	ip->daddr        = inet_addr(dst_addr);

	printf("IPPROTO_ICMP = %d, ICMP_ECHO = %d, PROTO_TYPE = %d\n", IPPROTO_ICMP, ICMP_ECHO, PROTO_TYPE);
	icmp->icmp_type = ICMP_ECHO;
	icmp->icmp_code = 0;
	icmp->icmp_id = htons(ID);
	icmp->icmp_seq = 1;
	memset(icmp->icmp_data, 0xa5, datalen);	/* fill with pattern */
	len = 8 + datalen;		/* checksum ICMP header and data */

	bzero(&socket_address.sll_addr, sizeof(struct sockaddr_ll));
	memcpy((void*)socket_address.sll_addr, (void*)dest_mac, 6);
	socket_address.sll_addr[6]  = 0x00;/*not used*/
	socket_address.sll_addr[7]  = 0x00;/*not used*/
	
	printf("socket_address.sll_addr: ");
	printHW(socket_address.sll_addr);
	printf("\n");
	
	//set socket_address values
	socket_address.sll_family   = PF_PACKET;	
	socket_address.sll_protocol = htons(PROTO_TYPE);	
	socket_address.sll_ifindex  = index;
	socket_address.sll_halen  =  ETH_ALEN;
	socket_address.sll_pkttype  = PACKET_OTHERHOST;
	
	//set up header
	memcpy((void*)buffer, (void*)dest_mac, ETH_ALEN);
	memcpy((void*)(buffer+ETH_ALEN), (void*)src_mac, ETH_ALEN);
	//memcpy((void*)data, (void*)packet, sizeof(packet));
	eh->h_proto = htons(PROTO_TYPE);

	icmp->icmp_cksum = 0;
	icmp->icmp_cksum = in_cksum((u_short *) icmp, len);
	ip->check = 0;
	ip->check = in_cksum((unsigned short *)ip, sizeof(struct iphdr));
	printf("ICMP Checksum: %d\n", icmp->icmp_cksum);
	printf("IP Chekcsum: %d\n", ip->check);
	//memcpy(packet + sizeof(struct iphdr), icmp, sizeof(struct icmp));
	
	connection.sin_family = AF_INET;
    connection.sin_addr.s_addr = inet_addr(dst_addr);
	
	//send the packet
	
	//verify(buffer, 0);
	
	if((sent = sendto(sockfd, buffer, ETH_FRAME_LEN, 0, (struct sockaddr*)&socket_address, sizeof(socket_address))) < 0)
	{
		printf("sendPacket sendto error: %d %s\n", errno, strerror(errno));
		return -1;
	}
	
	/*
	if((sent = sendto(sockfd, packet, sizeof(struct iphdr)  + sizeof(struct icmp), 0, (struct sockaddr*)&connection, sizeof(connection))) < 0)
	{
		printf("sendPacket sendto error: %d %s\n", errno, strerror(errno));
		return -1;
	}*/
	
	
	printf("Sent %d byte in ping from %s to %s \n", sent, src_addr, dst_addr);
	
	
	return 0;
}


int ping_send2(int request, int pg)
{
	int len, datalen = 8, sent;
	struct icmp	*icmp;
	struct iphdr *ip, *ipping;
	char* packet;
	char rcvline[ETH_FRAME_LEN];
	struct sockaddr_in connection;
	struct in_addr s;
	socklen_t addrlen = sizeof(connection);
	
	printf("Waiting-\n");
	if((len = recvfrom(request, rcvline, ETH_FRAME_LEN, 0, (struct sockaddr*) &connection, &addrlen)) < 0)
	{
			printf("pg recvfrom error: %d %s\n", errno, strerror(errno));
			return -1;
	}
	
	ipping = (struct ip*)(rcvline + 14);

	printf("Received ping from ");
	printHW(rcvline + ETH_ALEN);
	printf(" to ");
	printHW(rcvline);
	printf("\n");
	
	packet = malloc(sizeof(struct iphdr) + sizeof(struct icmp));
	ip = (struct iphdr*) packet;
	icmp = (struct icmp *) (packet + sizeof(struct iphdr));
	
	//data = buffer + 14;
	/*
	printf("Verifying Receving IP:\n");
	verify(rcvline, 0);
	printf("------\n");*/

	s.s_addr = ip->saddr;
	printf("Checking: %s ", inet_ntoa(s));
	s.s_addr = ip->daddr;
	printf(" to %s \n", inet_ntoa(s));
	/* ip header */
	ip->ihl          = 5;
	ip->version      = 4;
	ip->tos          = 0;
	ip->tot_len      = sizeof(struct iphdr)  + sizeof(struct icmp);
	ip->id           = htons(ID);
	ip->ttl          = 255;
	ip->protocol     = IPPROTO_ICMP;
	ip->saddr        = ipping->daddr;
	ip->daddr        = ipping->saddr;

	printf("IPPROTO_ICMP = %d, ICMP_ECHO = %d, PROTO_TYPE = %d\n", IPPROTO_ICMP, ICMP_ECHO, PROTO_TYPE);
	icmp->icmp_type = ICMP_ECHOREPLY;
	icmp->icmp_code = 0;
	icmp->icmp_id = htons(ID);
	icmp->icmp_seq = 1;
	memset(icmp->icmp_data, 0xa5, datalen);	/* fill with pattern */
	len = 8 + datalen;		/* checksum ICMP header and data */


	icmp->icmp_cksum = 0;
	icmp->icmp_cksum = in_cksum((u_short *) icmp, len);
	ip->check = 0;
	ip->check = in_cksum((unsigned short *)ip, sizeof(struct iphdr));
	printf("ICMP Checksum: %d\n", icmp->icmp_cksum);
	printf("IP Chekcsum: %d\n", ip->check);
	//memcpy(packet + sizeof(struct iphdr), icmp, sizeof(struct icmp));
	
	connection.sin_family = AF_INET;
    connection.sin_addr.s_addr = ip->daddr;
	
	//send the packet
	
	//verify(packet, 1);
	
	
	if((sent = sendto(pg, packet, sizeof(struct iphdr)  + sizeof(struct icmp), 0, (struct sockaddr*)&connection, sizeof(connection))) < 0)
	{
		printf("sendPacket sendto error: %d %s\n", errno, strerror(errno));
		return -1;
	}
	
	s.s_addr = ip->saddr;
	printf("Sent %d bytes in ping from %s ", sent, inet_ntoa(s));
	s.s_addr = ip->daddr;
	printf(" to %s \n", inet_ntoa(s));
	
	
	return 0;
}

int ping_recv(int pg)
{
	int				hlen1, icmplen;
	double			rtt;
	struct iphdr    *ip;
	struct icmp		*icmp;
	struct timeval	*tvsend;
	char *ptr, packet[ETH_FRAME_LEN];
	ssize_t len;
	struct msghdr *msg;
	struct timeval *tvrecv;
	struct sockaddr_in connection;
	socklen_t addrlen = sizeof(connection);
	
	msg = malloc(sizeof(struct msghdr));
	tvrecv = malloc(sizeof(struct timeval));
	tvsend = malloc(sizeof(struct timeval));
	
	printf("Waiting for ping reply...\n");
	if((len = recvfrom(pg, packet, ETH_FRAME_LEN, 0, (struct sockaddr*) &connection, &addrlen)) < 0)
	{
			printf("pg recvfrom error: %d %s\n", errno, strerror(errno));
			return -1;
	}
	
	printf("Received %d from %s\n", len, inet_ntoa(connection.sin_addr));
	ip = malloc(sizeof(struct iphdr));
	icmp = malloc(sizeof(struct icmp));
	ptr = packet;
	ip = (struct ip *) ptr;		/* start of IP header */
	hlen1 = ip->ihl << 2;		/* length of IP header */
	printf("got stuff\n");
	if (ip->protocol != IPPROTO_ICMP)
	{
		printf("ping_recv: wrong protocol\n");
		return -1;				/* not ICMP */
	}
    printf("It's ICMP\n");
	icmp = (struct icmp *) (ptr + hlen1);	/* start of ICMP header */
	if ( (icmplen = len - hlen1) < 8)
	{
		printf("ping_recv: malformed packet\n");
		return -1;				/* malformed packet */
	}
	printf("It's not malformed\n");
	if (icmp->icmp_type == ICMP_ECHOREPLY) 
	{
		if (ntohs(icmp->icmp_id) != ID)
		{
			printf("ping_recv: not a response to our ECHO_REQUEST\n");
			return;			/* not a response to our ECHO_REQUEST */
		}
		printf("It's our ECHOREPLY %d = %d\n", ntohs(icmp->icmp_id), ID);
		if (icmplen < 16)
		{
			printf("Not enough data\n");
			return;			/* not enough data to use */
		}
		
		printf("%d bytes from %s: seq=%u, ttl=%d, rtt=%.3f ms\n", icmplen, inet_ntoa(connection.sin_addr), icmp->icmp_seq, ip->ttl);
		return 0;
	}
}

/*
int main(int argc, char **argv)
{
	int pg, request, maxfd, index, optval = 1;
	struct hwa_info	*hwa;
	char src_addr[16], dest_addr[16], src_mac[6], dest_mac[6];
	fd_set fdset;
	
	printf("Starting\n");
	
	//request is used to send ICMP_ECHO packets
	request = socket(PF_PACKET, SOCK_RAW, htons(PROTO_TYPE));
	if(request < 0)
	{
		printf("request: %d %s\n", errno, strerror(errno));
		return -1;
	} 
	
	
	//pg socket is used to receive ICMP_ECHOREPLY
	pg = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if(pg < 0)
	{
		err_msg("pg sock: %d %s\n", errno, strerror(errno));
		return -1;
	}
	
	if(setsockopt(pg, IPPROTO_IP, IP_HDRINCL, &optval, sizeof(int)) < 0)
	{
		err_msg("set rt sock option: %d %s\n", errno, strerror(errno));
	}

	if(argc > 1)
	{

		
		printf("Interface stuff\n");
		for ( hwa = get_hw_addrs(); hwa != NULL; hwa = hwa->hwa_next)
		{
			if(strncmp(hwa->if_name, "eth0", 4) == 0)
			{
				printf("IP: %s\n", hwa->ip_addr);
				strncpy(src_addr, "192.168.1.107\0", 16);
				memcpy((void*)src_mac, hwa->if_haddr, 6);
				memcpy((void*)dest_mac, hwa->if_haddr, 6);
				strncpy(dest_addr, "192.168.1.108\0", 16);
				dest_mac[5] = dest_mac[5] + 1;
				index = hwa->if_index;
				break;
			}
		}
		
		printf("src: %s, MAC: ", src_addr);
		printHW(src_mac);
		printf(", interface: %d \n", index);
		printf("dest: %s, MAC: ", dest_addr);
		printHW(dest_mac);
		printf("\n");
		ping_send(request, src_addr, dest_addr, src_mac, dest_mac, index);
	}
	
	for(;;)
	{
		FD_ZERO(&fdset);
		FD_SET(pg, &fdset);
		FD_SET(request, &fdset);
		maxfd = max(pg, request) + 1;
		printf("Selecting...\n");
		if(select(maxfd, &fdset, NULL, NULL, NULL) < 0)
		{
			printf("select error: %d %s\n", errno, strerror(errno));
		}
		if(FD_ISSET(request, &fdset))
		{
			ping_send2(request, pg);
		}
		if(FD_ISSET(pg, &fdset))
		{
			ping_recv(pg);
		}
	
	}
	
	//ping_send(int sock, char* src_addr, char* dst_addr, char* src_mac, char* dest_mac, int index)
	//ping_send(request, src_addr, dest_addr, src_mac, dest_mac, index);
	//ping_send2(request, pg);
	//ping_send(pg, src_addr, dest_addr, src_mac, dest_mac, index);
	
	//ping_recv(int pg, ssize_t len, struct msghdr *msg, struct timeval *tvrecv)
	//ping_recv(pg);
}*/

void verify(char* buffer, int flag)
{
	struct icmp	*icmp;
	struct iphdr *ip;
	char* packet;
	unsigned char* etherhead; 
	struct ethhdr *eh;
	struct in_addr inaddr;
	
	if(flag == 0)
	{
		packet = buffer + 14;
		ip = (struct iphdr*) packet;
		icmp = (struct icmp *) (packet + sizeof(struct iphdr));

		etherhead = buffer;
		eh = (struct ethhdr *)etherhead;
		printf("Verifying: \n");
		printHW(buffer + ETH_ALEN);
		printf(" to ");
		printHW(buffer);
		printf("\n");
		printf("%d\n", ntohs(eh->h_proto));
	}
	else
	{
		packet = buffer;
		ip = (struct iphdr*) packet;
		icmp = (struct icmp *) (packet + sizeof(struct iphdr));
	}

	printf("IP: %d %d %d %d %d %d %d %d\n", ip->ihl, ip->version, ip->tos,	ip->tot_len, ip->id, ip->ttl, ip->protocol, ip->check);
	inaddr.s_addr = ip->saddr;
	printf("Source %s\n", inet_ntoa(inaddr));
	inaddr.s_addr = ip->daddr;
	printf("Destination %s\n", inet_ntoa(inaddr));
	printf("ICMP: %d %d %d %d %d\n", icmp->icmp_type, icmp->icmp_code, icmp->icmp_id, icmp->icmp_seq, icmp->icmp_cksum);

}

void printHW(char* ptr)
{
	int i = 6;
	do 
	{
		printf("%.2x%s", *ptr++ & 0xff, (i == 1) ? " " : ":");
	} while (--i > 0);
}
#include "a2h.h"
#include "unpifiplus.h"

void mydg_echo(int sockfd, struct sockaddr *pcliaddr, socklen_t clilen, struct sockaddr * myaddr);
void print_ifi(struct ifi_info	*ifi);

//Custom structure
struct sock_info 
{
	int	sockfd; //socket descriptor
	struct sockaddr  *bound_addr; //bound address
	struct sockaddr  *netmask_addr; //netmask address
	struct sockaddr  *subnet_addr;	//subnet address	
};

int main(int argc, char **argv)
{
	FILE 				*fp;
	int					i, j, k, match = 0, currentmax = 0, max_window_size = 0, lastsock = 0, maxfdp1 = 0, servport;
	int					socklist[MAXSOCKS];
	struct sock_info	infolist[MAXSOCKS];
	const int			on = 1;
	pid_t				pid;
	struct ifi_info		*ifi, *ifihead;
	socklen_t 			clilen;
	struct sockaddr_in	*sa;
	struct sockaddr 	*rcv, *cliaddr;
	fd_set 				rset;
	char				mesg[MAXLINE], temp_subnet[14];

	//Read information from server.in file
	fp = fopen("server.in", "r");
	
	if (fp == NULL) 
	{
		printf("Error opening server.in\n");
		exit(1);
	}
	
	fscanf(fp, "%d", &servport);
	fscanf(fp, "%d", &max_window_size);

	printf("Port: %d, Max Window Size: %d\n", servport, max_window_size);
	

	for (i = 0, ifihead = ifi = Get_ifi_info_plus(AF_INET, 1); ifi != NULL && i < MAXSOCKS; ifi = ifi->ifi_next, i++) 
	{
		//4bind unicast address 
		socklist[i] = socket(AF_INET, SOCK_DGRAM, 0);

		setsockopt(socklist[i], SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

		sa = (struct sockaddr_in *) ifi->ifi_addr;
		sa->sin_family = AF_INET;
		sa->sin_port = htons(servport);
		bind(socklist[i], (struct sockaddr *) sa, sizeof(*sa));
		
		//Store information in customized structure
		infolist[i].sockfd = socklist[i];
		infolist[i].bound_addr = ifi->ifi_addr;
		infolist[i].netmask_addr = ifi->ifi_ntmaddr;
		infolist[i].subnet_addr = malloc(sizeof(struct sockaddr));
		
		//Subnet = Address & Netmask
		for(j = 0; j < 14; j++)
		{
			infolist[i].subnet_addr->sa_data[j] = ifi->ifi_addr->sa_data[j] & ifi->ifi_ntmaddr->sa_data[j];
		}
		
		//Print IP, netmask, and subnet
		printf("bound : \n");
		sa = (struct sockaddr_in *) infolist[i].bound_addr;
		printf("  IP addr: %s\n", Sock_ntop_host(sa, sizeof(*sa)));
		sa = (struct sockaddr_in *) infolist[i].netmask_addr;
		printf("  Netmask addr: %s\n", Sock_ntop_host(sa, sizeof(*sa)));
		sa = (struct sockaddr_in *) infolist[i].subnet_addr;
		printf("  Subnet addr: %s\n\n", Sock_ntop_host(sa, sizeof(*sa)));
		
		lastsock = i;
		//printf("bound %s\n", sock_ntop((struct sockaddr *) sa, sizeof(*sa)));
		//print_ifi(ifi);
	}
	
	FD_ZERO(&rset);
	clilen = sizeof(cliaddr);
		
	//Set up infinite loop for handling incoming connections
	for ( ; ; ) 
	{
		//Set up file descriptors for select()
		for(i = 0; i < lastsock; i++)
		{
			FD_SET(socklist[i], &rset);
		}
		
		for(i = 0; i < lastsock; i++)
		{
				currentmax = max(currentmax, socklist[i]);
		}
		
		maxfdp1 = currentmax + 1;
		select(maxfdp1, &rset, NULL, NULL, NULL);
		
		//Handle a connection request on the first socket
		for(i = 0; i < lastsock; i++)
		{
			if(FD_ISSET(socklist[i], &rset))
			{
				if ( (pid = Fork()) == 0) 
				{	
					for(j = 0; j < i; j++)
					{
						close(socklist[j]);
					}
					
					for(j = i + 1; j < lastsock; j++)
					{
						close(socklist[j]);
					}
					
					j = recvfrom(socklist[i], mesg, MAXLINE, 0, cliaddr, &clilen);
					rcv = (struct sockaddr *) cliaddr;
					if(strcmp(Sock_ntop_host(rcv, sizeof(*rcv)), "127.0.0.1") == 0)
					{
						printf("Client address is the loopback address\n");
					}
					else
					{
						for(j = 0; j < lastsock; j++)
						{
							for(k = 0; k < 14; k++)
							{
								temp_subnet[k] = rcv->sa_data[k] & infolist[j].netmask_addr->sa_data[k];
							}
							
							match = 1;
							for(k = 0; k< 14; k++)
							{
								if(temp_subnet[k] != infolist[j].subnet_addr->sa_data[k])
									match = 0;
								break;
							}
							
							if(match == 1)
							{
								printf("Client: %s\n Server: %s\n Subnet: %s\n", Sock_ntop_host(rcv, sizeof(*rcv)), infolist[j].bound_addr, temp_subnet);
								break;
							}
						}				
					}
					
					//mydg_echo(socklist[i], (struct sockaddr *) &cliaddr, sizeof(cliaddr), (struct sockaddr *) sa);
					exit(0);		// never executed
				}
			}
		}		
	}
	/*
	if ( (pid = Fork()) == 0) 
	{	
		// child 
		mydg_echo(sockfd, (SA *) &cliaddr, sizeof(cliaddr), (SA *) sa);
		exit(0);		// never executed
	}*/
}

//Used for testing, will be removed before submission 
void mydg_echo(int sockfd, struct sockaddr *pcliaddr, socklen_t clilen, struct sockaddr *myaddr)
{
	int			n;
	char		mesg[MAXLINE];
	socklen_t	len;

	for ( ; ; ) {
		len = clilen;
		n = recvfrom(sockfd, mesg, MAXLINE, 0, pcliaddr, &len);
		printf("child %d, datagram from %s", getpid(),
			   Sock_ntop(pcliaddr, len));
		printf(", to %s\n", Sock_ntop(myaddr, clilen));

		sendto(sockfd, mesg, n, 0, pcliaddr, len);
	}
}

//Used for testing, will be removed before submission
void print_ifi(struct ifi_info		*ifi)
{
	struct sockaddr	*sa;
	u_char		*ptr;
	int		i;
	
	if (ifi->ifi_index != 0)
			printf("(%d) ", ifi->ifi_index);
		printf("<");
/* *INDENT-OFF* */
		if (ifi->ifi_flags & IFF_UP)			printf("UP ");
		if (ifi->ifi_flags & IFF_BROADCAST)		printf("BCAST ");
		if (ifi->ifi_flags & IFF_MULTICAST)		printf("MCAST ");
		if (ifi->ifi_flags & IFF_LOOPBACK)		printf("LOOP ");
		if (ifi->ifi_flags & IFF_POINTOPOINT)	printf("P2P ");
		printf(">\n");
/* *INDENT-ON* */

		if ( (i = ifi->ifi_hlen) > 0) {
			ptr = ifi->ifi_haddr;
			do {
				printf("%s%x", (i == ifi->ifi_hlen) ? "  " : ":", *ptr++);
			} while (--i > 0);
			printf("\n");
		}
		if (ifi->ifi_mtu != 0)
			printf("  MTU: %d\n", ifi->ifi_mtu);

		if ( (sa = ifi->ifi_addr) != NULL)
			printf("  IP addr: %s\n",
						Sock_ntop_host(sa, sizeof(*sa)));

/*=================== cse 533 Assignment 2 modifications ======================*/

		if ( (sa = ifi->ifi_ntmaddr) != NULL)
			printf("  network mask: %s\n",
						Sock_ntop_host(sa, sizeof(*sa)));

/*=============================================================================*/

		if ( (sa = ifi->ifi_brdaddr) != NULL)
			printf("  broadcast addr: %s\n",
						Sock_ntop_host(sa, sizeof(*sa)));
		if ( (sa = ifi->ifi_dstaddr) != NULL)
			printf("  destination addr: %s\n",
						Sock_ntop_host(sa, sizeof(*sa)));
}
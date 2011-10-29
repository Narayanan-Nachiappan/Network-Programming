#include "unp.h"
#include "a2h.h"
#include "struct.c"
#include "unpifiplus.h"

void sig_chld(int signo);
void mydg_echo(int sockfd, struct sockaddr *pcliaddr, socklen_t clilen, struct sockaddr * myaddr);
void print_ifi(struct ifi_info	*ifi);
//void dg_echofun(FILE * fp,int sockfd, const SA *pcliaddr, socklen_t clilen);
//void dg_echofun(FILE * fp,int sockfd, const SA *pcliaddr, socklen_t clilen);
//ssize_t dg_send_recv(int fd, const void *outbuff, size_t outbytes, void *inbuff, size_t inbytes, const SA *destaddr, socklen_t destlen)

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
	int					i, j, k, match = 0, currentmax = 0, max_window_size = 0, numsocks = 0, maxfdp1 = 0;
	int					connsock, servport, ephemeral, flags, local = 0;
	int					socklist[MAXSOCKS];
	struct sock_info	infolist[MAXSOCKS];
	const int			on = 1;
	pid_t				pid;
	struct ifi_info		*ifi, *ifihead;
	socklen_t 			clilen, clilen2, servlen;
	struct sockaddr_in	*sa, cliaddr, cliaddr2, *servaddr;
	struct sockaddr 	*rcv;
	fd_set 				rset;
	char				filename[MAXLINE], rcvline[MAXLINE], sendline[MAXLINE], portnum[10], address[16];

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
	
	//Bind unicast address 
	for (i = 0, ifihead = ifi = Get_ifi_info_plus(AF_INET, 1); ifi != NULL && i < MAXSOCKS; ifi = ifi->ifi_next, i++) 
	{
		socklist[i] = socket(AF_INET, SOCK_DGRAM, 0);
		setsockopt(socklist[i], SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
		
		/*
		if((flags = fcntl(socklist[i], F_GETFL, 0)) < 0)
		printf("F_GETFL Error socklist[%d]\n", i);
		flags |= O_NONBLOCK;
		if(fcntl(socklist[i], F_SETFL, flags) < 0)
		printf("F_SETFL Error socklist[%d]\n", i);*/

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
		printf("  IP address: %s\n", Sock_ntop_host(infolist[i].bound_addr, sizeof(*infolist[i].bound_addr)));
		printf("  Netmask address: %s\n", Sock_ntop_host(infolist[i].netmask_addr, sizeof(*infolist[i].netmask_addr)));
		printf("  Subnet address: %s\n\n", Sock_ntop_host(infolist[i].subnet_addr, sizeof(*infolist[i].subnet_addr)));
		
		numsocks = i + 1;
		//print_ifi(ifi);
	}
	
	printf("Found %d interfaces\n", numsocks);

	//Set up SIGCHLD handler
	signal(SIGCHLD, sig_chld);
	
	FD_ZERO(&rset);
	clilen = sizeof(cliaddr);
		
	//Set up infinite loop for handling incoming connections
	//printf("Infinite loop\n");
	for ( ; ; ) 
	{
		//Set up file descriptors for select()
		for(i = 0; i < numsocks; i++)
		{
			FD_SET(socklist[i], &rset);
		}
		//printf("FD_SET\n");
		currentmax = 0;
		for(i = 0; i < numsocks; i++)
		{
				currentmax = max(currentmax, socklist[i]);
				//printf("currentmax = %d\n", currentmax);
		}
		
		maxfdp1 = currentmax + 1;
		printf("maxfdp1 = %d\n", maxfdp1);
		
		printf("select\n");
		select(maxfdp1, &rset, NULL, NULL, NULL);
		
		//Handle a connection request on sockets that are ready
		printf("selected\n");
		for(i = 0; i < numsocks; i++)
		{
			if(FD_ISSET(socklist[i], &rset))
			{
				//Fork child to handle client
				printf("Socket %d ready\n", i);
				if ((pid = Fork()) == 0) 
				{	
					//Close all other sockets
					for(j = 0; j < i; j++)
					{
						close(socklist[j]);
					}
					
					for(j = i + 1; j < numsocks; j++)
					{
						close(socklist[j]);
					}
					
					//Get filename from client
					printf("Recvfrom\n");
					//******************NANA! Is there any function that just waits to receive one message and then sends an ack? If so, let me know
					//and I can change this part to include it
					//j = recvfrom(socklist[i], filename, MAXLINE, 0, (struct sockaddr *)&cliaddr, &clilen);
					j = recvfrom(socklist[i], (struct message *)&recv_msg, sizeof(recv_msg), 0,  (struct sockaddr *)&cliaddr, &clilen);
					if(j == -1)
					{
						printf("recvfrom error %d\n, exiting", errno); //recvfrom error
						exit(1);
					}
					
					//strcpy(address, );
					//rcv = (struct sockaddr *) cliaddr;
					inet_ntop(AF_INET, &cliaddr.sin_addr, address, clilen);
					printf("Received from %s, filename is %s, received %d bytes\n", address,  recv_msg.data, j);
					//printf("Received from %s, filename is %s, received %d bytes\n", address,  filename, j);
					
					//test to see if the client is local
					if(strcmp(address, "127.0.0.1") == 0) //don't test for localhost
					{
						printf("Client is LOCAL (loopback address)\n");
					}
					else
					{						
						if((local = isLocal((struct sockaddr*)&cliaddr, infolist[i].bound_addr, infolist[i].netmask_addr)) == 0)
							printf("Client is LOCAL - ");
						else
							printf("Client is NOT LOCAL - ");
						
						printf("Client: %s, ", Sock_ntop_host((struct sockaddr*)&cliaddr, sizeof(*(struct sockaddr*)&cliaddr)));
						printf("Server: %s\n", Sock_ntop_host(infolist[i].bound_addr, sizeof(*infolist[i].bound_addr)));
					}

					//Create connection socket and set socket options
					connsock = socket(AF_INET, SOCK_DGRAM, 0);
					setsockopt(connsock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
					if(local == 0)
						setsockopt(connsock, SOL_SOCKET, SO_DONTROUTE, &on, sizeof(on));

					bzero(&servaddr, sizeof(servaddr));
					servaddr = (struct sockaddr_in *) infolist[i].bound_addr;
					servaddr->sin_family      = AF_INET;
					servaddr->sin_port       = htons(0);
					
					//Bind to correct IPServer and port 0
					printf("Binding connection socket to %s\n", Sock_ntop((struct sockaddr *) servaddr, sizeof(*(struct sockaddr *)servaddr)));
					if(bind(connsock, (struct sockaddr *) servaddr, sizeof(*(struct sockaddr *) servaddr)) < 0)
					{
						printf("bind connosck error, %d\n", errno);
						exit(1);
					}
					
					//Use getsockname to get ephemeral port number
					printf("getsockname\n");
					servlen = sizeof(*(struct sockaddr *) servaddr);
					if(getsockname(connsock, (struct sockaddr *) servaddr, &servlen) < 0)
					{
						printf("getsockname error, %d\n", errno);
						exit(1);
					}
					
					//Connect to connection socket
					printf("Ephemeral Port: %d\n", ntohs(servaddr->sin_port));
					cliaddr2 = cliaddr;
					cliaddr2.sin_port = servaddr->sin_port;
					clilen2 = sizeof(cliaddr2);
					if(connect(connsock, (struct sockaddr *) &cliaddr2, clilen2) < 0)
					{
						printf("connect connsock error, %d\n", errno);
						exit(1);
					}
					
					//Send ephemeral port number to client
					printf("Connsock connected! Sending ephemeral port number\n");
					sprintf(portnum, "%d", ntohs(servaddr->sin_port));
					
					//ssize_t dg_send_recv(int fd, const void *outbuff, size_t outbytes, void *inbuff, size_t inbytes, const SA *destaddr, socklen_t destlen)
					//sendto(socklist[i], sendline, strlen(sendline), 0, (struct sockaddr *)&cliaddr, clilen);
					
					send_msg.type = 2;
					strcpy(send_msg.data, portnum);
					sendto(socklist[i],(struct message *)&send_msg, sizeof(send_msg), 0, (struct sockaddr *)&cliaddr, clilen);
					//dg_send_recv(socklist[i],(struct message *)&send_msg, sizeof(send_msg), 0, (struct sockaddr *)&cliaddr, clilen);
					//dg_send_recv(socklist[i], portnum, strlen(portnum), 0, (struct sockaddr *)&cliaddr, clilen, 2);
					//**************************NANA! Am I using this correctly? I'm sending the port number as a string
					//dg_sendrecv(socklist[i], msgrecv, 0 sendline, strlen(sendline), (struct sockaddr *) &cliaddr, clilen);					
					close(socklist[i]);
					
					//Start file transfer
					//*************************NANA! This is where the file transfer starts
					strncpy(filename, recv_msg.data, strlen(recv_msg.data));
					printf("Sending file: %s\n", filename);
					
					fp = fopen(filename, "r");
					
					if(fp == NULL)
					{
						printf("Server child error: file open\n");
						exit(1);
					}
					/*
					while (Fgets(sendline, MAXLINE, fp) != NULL) 
					{
						//Fputs(sendline,stdout);
						fprintf(stderr, "%s", sendline);
					}*/
					dg_echofun(fp, connsock, (struct sockaddr *)&cliaddr2, clilen2);
					
					exit(0);
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

//SIGCHLD Handler p138
void sig_chld(int signo)
{
	pid_t pid;
	int stat;
	
	while((pid = waitpid(-1, &stat, WNOHANG)) > 0) //Wait for the correct child to terminate
		printf("SIGCHLD - child %d terminated\n\n", pid);
	return;
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
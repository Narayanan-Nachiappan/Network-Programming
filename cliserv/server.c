#include "unp.h"
#include "a2h.h"
#include "struct.c"
#include "unpifiplus.h"

void sig_chld(int signo);

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
	int					connsock, servport, ephemeral, flags, local = 0, windowtype = 1;
	int					socklist[MAXSOCKS];
	struct sock_info	infolist[MAXSOCKS];
	const int			on = 1;
	pid_t				pid;
	struct ifi_info		*ifi, *ifihead;
	socklen_t 			clilen, clilen2, servlen;
	struct sockaddr_in	*sa, cliaddr, cliaddr2, *servaddr;
	fd_set 				rset;
	char				filename[MAXLINE], rcvline[MAXLINE], sendline[MAXLINE], portnum[10], address[16];

	//Read information from server.in file
	printf("Reading server.in\n");
	fp = fopen("server.in", "r");
	if (fp == NULL) 
	{
		printf("Error opening server.in\n");
		exit(1);
	}
	fscanf(fp, "%d", &servport);
	fscanf(fp, "%d", &max_window_size);
	fscanf(fp, "%d", &windowtype);
	
	printf("Port: %d, Max Window Size: %d\n", servport, max_window_size);
	if(windowtype == 1)
		printf("ARQ Type: Stop-and-Wait\n");
	else
		printf("ARQ Type: Selective ARQ\n");
	
	//bind all unicast addresses 
	for (i = 0, ifihead = ifi = Get_ifi_info_plus(AF_INET, 1); ifi != NULL && i < MAXSOCKS; ifi = ifi->ifi_next, i++) 
	{
		socklist[i] = socket(AF_INET, SOCK_DGRAM, 0);
		setsockopt(socklist[i], SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
		
		if((flags = fcntl(socklist[i], F_GETFL, 0)) < 0)
		printf("F_GETFL Error socklist[%d]\n", i);
		flags |= O_NONBLOCK;
		if(fcntl(socklist[i], F_SETFL, flags) < 0)
		printf("F_SETFL Error socklist[%d]\n", i);

		sa = (struct sockaddr_in *) ifi->ifi_addr;
		sa->sin_family = AF_INET;
		sa->sin_port = htons(servport);
		
		//bind on the sockets
		if(bind(socklist[i], (struct sockaddr *) sa, sizeof(*sa)) < 0)
			printf("bind failed, error %d %s\n", errno, strerror(errno));
		else
		{
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
		}
	}
	
	printf("Found %d interfaces\n", numsocks);

	//Set up SIGCHLD handler
	signal(SIGCHLD, sig_chld);
		
	//Set up infinite loop for handling incoming connections
	for ( ; ; ) 
	{
		//Set up file descriptors for select()
		FD_ZERO(&rset);
		for(i = 0; i < numsocks; i++)
		{
			FD_SET(socklist[i], &rset);
		}

		currentmax = 0;
		for(i = 0; i < numsocks; i++)
		{
				currentmax = max(currentmax, socklist[i]);
		}
		
		maxfdp1 = currentmax + 1;

		if(select(maxfdp1, &rset, NULL, NULL, NULL) < 0)
		{
			if(errno != 4)
				printf("select error %d %s\n", errno, strerror(errno));
		}
		
		//Handle a connection request on sockets that are ready
		for(i = 0; i < numsocks; i++)
		{
			if(FD_ISSET(socklist[i], &rset))
			{
				//Fork child to handle client
				//Get filename from client
				clilen = sizeof(cliaddr);
				j = recvfrom(socklist[i], (struct message *)&recv_msg, sizeof(recv_msg), 0,  (struct sockaddr *)&cliaddr, &clilen);
				inet_ntop(AF_INET, &cliaddr.sin_addr, address, clilen);
				
				//if it's a valid request, fork a child. Otherwise, continue checking
				if(j == -1)
					continue;
				if(recv_msg.type == 1)
					printf("Received from %s, filename is %s, received %d bytes\n", address,  recv_msg.data, j);
				else 
					continue;
				
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
					
					//test to see if the client is local
					inet_ntop(AF_INET, &cliaddr.sin_addr, address, clilen);			
					
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
					printf("binding connection socket to %s\n", Sock_ntop((struct sockaddr *) servaddr, sizeof(*(struct sockaddr *)servaddr)));
					if(bind(connsock, (struct sockaddr *) servaddr, sizeof(*(struct sockaddr *) servaddr)) < 0)
					{
						printf("bind connosck error, %d\n", errno);
						exit(1);
					}
					
					//Use getsockname to get ephemeral port number
					servlen = sizeof(*(struct sockaddr *) servaddr);
					if(getsockname(connsock, (struct sockaddr *) servaddr, &servlen) < 0)
					{
						printf("getsockname error, %d\n", errno);
						exit(1);
					}
					
					//Connect to connection socket
					printf("Ephemeral Port: %d\n", servaddr->sin_port);
					printf("Connect:");
					printf("IP Address : %s\n",Inet_ntop(AF_INET, &cliaddr.sin_addr, rcvline, sizeof(rcvline)));
					printf("Well-known port number : %d\n", cliaddr.sin_port);
					
					if(connect(connsock, (struct sockaddr *) &cliaddr, clilen) < 0)
					{
						printf("connect connsock error, %d\n", errno);
						exit(1);
					}
					
					//Send ephemeral port number to client
					printf("Connsock connected! Sending ephemeral port number\n");
					sprintf(portnum, "%d", servaddr->sin_port);
					
					strcpy(send_msg.data, portnum);
					dg_send_recv(socklist[i], connsock, portnum, strlen(portnum), NULL, NULL, (struct sockaddr *)&cliaddr, clilen, 2, 0);				
					close(socklist[i]);
					
					strncpy(filename, recv_msg.data, strlen(recv_msg.data));
					printf("Sending file: %s\n", filename);
					
					fp = fopen(filename, "r");
					
					if(fp == NULL)
					{
						printf("Server child error: file open\n");
						exit(1);
					}
					
					dg_echofun(fp, connsock, (struct sockaddr *)&cliaddr, clilen, windowtype, max_window_size);
					close(connsock);					
					exit(0);
				}
			}
		}		
	}
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
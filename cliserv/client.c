#include "unpifiplus.h"
#include "a.h"
#include "unprtt.h"
#include "setjmp.h"
#include <stdio.h>
#include <stdlib.h>

void send_cli(FILE*, int, const SA*, socklen_t);
static void sig_alrm(int);

static struct rtt_info rttinfo;
static sigjmp_buf jmpbuf;
static int rttinit = 0;

int main(int argc, char **argv){
	Fputs("CSE 533 : Network Programming\n",stdout);
	err_msg("Amelia Ellison - 107838108");
	err_msg("Narayanan Nachiappan - 107996031");
	err_msg("Youngbum Kim - 107387376");
	err_msg("Assignment #2");
	err_msg("----------------------------------------");
	err_msg("client.c");
	err_msg("----------------------------------------");
	err_msg("Input (client.in)");

	FILE *clientin;
	clientin=fopen ("client.in","r");
	char serv_ip_addr[INET_ADDRSTRLEN];
	char cli_ip_addr[INET_ADDRSTRLEN];
	char local_cli_ip_addr[INET_ADDRSTRLEN];
	char port[5];
	char file_name[20];
	char window_size[20];
	char seed[20];
	char loss[20];
	char mean[20];
	char recvline[MAXLINE];
	
	fscanf(clientin,"%s", serv_ip_addr);
	fscanf(clientin,"%s", port);
	fscanf(clientin,"%s", file_name);
	fscanf(clientin,"%s", window_size);
	fscanf(clientin,"%s", seed);
	fscanf(clientin,"%s", loss);
	fscanf(clientin,"%s", mean);

	fclose(clientin);

	err_msg("IP Address of the server: %s", serv_ip_addr);
	err_msg("Well known port number of server: %s", port);
	err_msg("File name to be transferred: %s", file_name);
	err_msg("Receiving sliding-window size: %s", window_size);
	err_msg("Random generator seed value: %s", seed);
	err_msg("Probability p of datagram loss: %s", loss);
	err_msg("The mean u in milliseconds: %s", mean);
	err_msg("----------------------------------------");

	/* copied from prifinfo_plus.c
	   examine client/server IP Addresses */

	struct ifi_info	*ifi, *ifihead;
	struct sockaddr	*sa;
	u_char *ptr;
	int i;
	int isLocal = 0, isSameHost = 0;
	char bc_addr[INET_ADDRSTRLEN];

	err_msg("Examine Client's IP Address: 127.0.0.1");
	
	for (ifihead = ifi = Get_ifi_info_plus(AF_INET, atoi("127.0.0.1")); ifi != NULL; ifi = ifi->ifi_next) {
		
		printf("%s: ", ifi->ifi_name);
		if (ifi->ifi_index != 0)
			printf("(%d) ", ifi->ifi_index);
		printf("<");
/* *INDENT-OFF* */
		if (ifi->ifi_flags & IFF_UP)			printf("UP ");
		if (ifi->ifi_flags & IFF_BROADCAST)		printf("BCAST ");
		if (ifi->ifi_flags & IFF_MULTICAST)		printf("MCAST ");
		if (ifi->ifi_flags & IFF_LOOPBACK)		printf("LOOP ");		
		else { // if it's not a lookback address
			if ( (sa = ifi->ifi_addr) != NULL){
				// set the client IP Address
				// if the client has multiple IP addresses, the client IP will be set as the last one
				// if the server is the same host as the client, this will be set as the lookback address later
				strcpy(cli_ip_addr, Sock_ntop_host(sa, sizeof(*sa)));
				
				if(!isLocal){
					char network_mask[20];
					strcpy(network_mask, Sock_ntop_host(ifi->ifi_ntmaddr, sizeof(*sa)));
					if(isLocalNetwork(cli_ip_addr, serv_ip_addr, network_mask) == 0){
						strcpy(local_cli_ip_addr, Sock_ntop_host(sa, sizeof(*sa)));
						isLocal = 1;
					}
				}
				if(!isSameHost){
					if(strcmp(Sock_ntop_host(sa, sizeof(*sa)), serv_ip_addr) == 0
						|| strcmp(serv_ip_addr, "127.0.0.1") == 0
						) isSameHost = 1;
				}
			}
		}
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

		if ( (sa = ifi->ifi_brdaddr) != NULL){
			printf("  broadcast addr: %s\n",
						Sock_ntop_host(sa, sizeof(*sa)));
			strcpy(bc_addr, Sock_ntop_host(sa, sizeof(*sa)));
		}
		if ( (sa = ifi->ifi_dstaddr) != NULL)
			printf("  destination addr: %s\n",
						Sock_ntop_host(sa, sizeof(*sa)));
	}
	
	err_msg("Server address: %s", serv_ip_addr);
	free_ifi_info_plus(ifihead);

	err_msg("----------------------------------------");
	
	if(isSameHost){
		err_msg("Server is the same host as the client,");
		err_msg(" change the server address to loopback address, so as client address.");
		strcpy(serv_ip_addr, "127.0.0.1");
		strcpy(cli_ip_addr, "127.0.0.1");
	} else if(isLocal){
		err_msg("Server is local");
		strcpy(cli_ip_addr, local_cli_ip_addr);
	}
	
	err_msg("Client address: %s", cli_ip_addr);
	err_msg("Server address: %s", serv_ip_addr);
	serv_ip_addr[15] = '\0';
	int					sockfd;
	struct sockaddr_in	servaddr;

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	
	// use 0 as the port number. this will cause the kernel to bind an ephemeral port to the socket.
	servaddr.sin_port = htons(0);
	sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

	// added
	err_msg("----------------------------------------");
	err_msg("Bind a socket");
	
	//Bind(sockfd, (SA *) &servaddr, sizeof(servaddr));
	if (bind(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0){
		err_quit("bind error %d", errno);
	};
	
	struct sockaddr_in ss_cli, ss_serv;
	char str[INET_ADDRSTRLEN];

	socklen_t len;
	len = sizeof(ss_cli);
	if (getsockname(sockfd, (SA *) &ss_cli, &len) < 0){
		err_quit("getsockname error %d", errno);
	}
	err_msg("IP Client");
	err_msg("Ephemeral port number assigned : %d", ss_cli.sin_port);
	
	err_msg("----------------------------------------");
	err_msg("Connect a socket");
	

	Inet_pton(AF_INET, serv_ip_addr, &servaddr.sin_addr);
	servaddr.sin_port = htons(atoi(port));

	if(connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0)
	{
		printf("connect error %d\n", errno);
		exit(1);
	}

	len = sizeof(ss_serv);
	if (getpeername(sockfd, (SA *) &ss_serv, &len) < 0){
		err_quit("getpeername error %d", errno);
	}
	
	err_msg("IP Server");
	err_msg("IP Address : %s",Inet_ntop(AF_INET, &ss_serv.sin_addr, str, sizeof(str)));
	err_msg("Well-known port number : %d",ss_serv.sin_port);
	
	err_msg("----------------------------------------");
	err_msg("Initializing connection.");
	err_msg("Filename : %s", file_name);
	
	Signal(SIGALRM, sig_alrm);
	rtt_newpack(&rttinfo); /* initialize for this packet */

	if (rttinit == 0) {
		rtt_init(&rttinfo); /* first time we're called */
		rttinit = 1;
		rtt_d_flag = 1;
	}
	send_msg = messageFactory(HD_INIT_CLI, file_name);
	send_msg.ts = rtt_ts(&rttinfo);

sendagain:
	err_msg("Client: send file name");
	Writen(sockfd, (char *)&send_msg, sizeof(send_msg));
	alarm(rtt_start(&rttinfo));

	if (sigsetjmp(jmpbuf, 1) != 0) {
		if (rtt_timeout(&rttinfo) < 0) {
			err_msg("Client: no response from server. terminates.");
			rttinit = 0; /* reinit in case we're called again */
			errno = ETIMEDOUT;
			return(-1);
		}
		#ifdef RTT_DEBUG
			err_msg("client: timeout, retransmitting");
		#endif
		goto sendagain;
	}
		do{
			err_msg("Waiting for initial packet from the server.");
			Recvfrom(sockfd, (struct message *)&recv_msg, MAXLINE, 0, NULL,NULL);
			printMessage(recv_msg);

			if(recv_msg.type != HD_INIT_SERV){
				err_msg("Different protocol type.");
			}
		} while(recv_msg.type != HD_INIT_SERV);
	
		alarm(0);
	//change socket number and make another connection.
	servaddr.sin_port = htons(getIntMsg(recv_msg));

	err_msg("----------------------------------------");
	err_msg("Reconnect:");
	err_msg("IP Address : %s",Inet_ntop(AF_INET, &servaddr.sin_addr, str, sizeof(str)));
	err_msg("Well-known port number : %d", servaddr.sin_port);
	
	if(connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0){
		printf("connect failed! Errno = %d\n", errno);
		exit(1);
	}

	err_msg("----------------------------------------");
	err_msg("Send ack to the server for initial connection");
	send_msg = messageFactory(HD_INIT_ACK, "I'm ready!"); // send ack to the server
	send_msg.seq=recv_msg.seq;
	Writen(sockfd, (char *)&send_msg, sizeof(send_msg));

	pthread_t tid;
	Pthread_create(&tid, NULL, printBuffer, atoi(mean));

	dg_client( sockfd, (SA *) &servaddr, sizeof(servaddr),atoi(window_size));
	
	while(headPtr != NULL){
		err_msg("Wait until recv_buff gets empty");
		sleep(1);
	}

	exit(0);
}

static void sig_alrm(int signo){
	siglongjmp(jmpbuf, 1);
}

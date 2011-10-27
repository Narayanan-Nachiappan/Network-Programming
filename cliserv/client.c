#include "unp.h"
#include "unpifiplus.h"
#include "a.h"
#include <stdio.h>
#include <stdlib.h>

struct message initialConnect(FILE*, int, const SA*, socklen_t, char*);
void send_cli(FILE*, int, const SA*, socklen_t);

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
	servaddr.sin_port = htons(0);
	Inet_pton(AF_INET, serv_ip_addr, &servaddr.sin_addr);

	sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

	// added
	err_msg("----------------------------------------");
	err_msg("Bind a socket");
	
	if (bind(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0){
		err_quit("bind error %d", errno);	
	};
	
	struct sockaddr_in ss_cli, ss_serv;
	char str[INET_ADDRSTRLEN];

	socklen_t len;
	len = sizeof(ss_cli);
	if (getsockname(sockfd, &ss_cli, &len) < 0){
		err_quit("getsockname error %d", errno);
	}
	err_msg("IP Client");
	err_msg("IP Address : %s", Inet_ntop(AF_INET, &ss_cli.sin_addr, str, sizeof(str)));
	err_msg("Ephemeral port number assigned : %d", ss_cli.sin_port);

	err_msg("----------------------------------------");
	err_msg("Connect a socket");
	
	servaddr.sin_port = htons(atoi(port));

	Connect(sockfd, (SA *) &servaddr, sizeof(servaddr));

	len = sizeof(ss_serv);
	if (getpeername(sockfd, &ss_serv, &len) < 0){
		err_quit("getpeername error %d", errno);
	}
	
	Fputs("IP Server\n",stdout);
	Fputs("IP Address : ",stdout);
	Fputs(Inet_ntop(AF_INET, &ss_serv.sin_addr, str, sizeof(str)) ,stdout);
	Fputs("\n",stdout);
	Fputs("Well-known port number : ",stdout);
	err_msg("%d", ss_serv.sin_port);
	
	////////
	Fputs("----------------------------------------\n",stdout);
	Fputs("Initializing connection: send filename\n",stdout);
	Fputs(file_name,stdout);
	
	// loop

	initialConnect(stdin, sockfd, (SA *) &servaddr, sizeof(servaddr), file_name);


	if(isTypeOf(recvmsg, HD_INIT_SERV) > 0){
		printf("msg : %s", recvmsg.data);
	}

	//

	// change socket number and make another connection.

	//send_cli(stdin, sockfd, (SA *) &servaddr, sizeof(servaddr));

	exit(0);
}

struct message initialConnect(FILE *fp, int sockfd, const SA *pservaddr, socklen_t servlen, char* filename){
	int n;
	char	sendline[MAXLINE], recvline[MAXLINE + 1];

	sendmsg = messageFactory(HD_INIT,"Working");

	Sendto(fd, (char *) &msg, sizeof(msg), 0, &pservaddr, sizeof(pservaddr));
	//Writen(sockfd, sendline, strlen(sendline));
	
	n = Recvfrom(fd, (char *)&recvmsg, MAXLINE, 0,  NULL, NULL);
	//n = Recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);

	return recvmsg;
}

void send_cli(FILE *fp, int sockfd, const SA *pservaddr, socklen_t servlen){
	int	n;
	char	sendline[MAXLINE], recvline[MAXLINE + 1];

	while (Fgets(sendline, MAXLINE, fp) != NULL) {
		// changed Sendto to Writen
		Writen(sockfd, sendline, strlen(sendline));
		n = Recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);

		recvline[n] = 0;	/* null terminate */
		Fputs(recvline, stdout);
	}
}

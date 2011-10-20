#include "unp.h"
#include "code/unpifiplus.h"
#include "code/get_ifi_info_plus.c"
#include <stdio.h>
#include <string.h>

int isLocalNetwork(char*, char*, char*);
void send_cli(FILE*, int, const SA*, socklen_t);

int main(int argc, char **argv){
	err_msg("CSE 533 : Network Programming");
	err_msg("Amelia Ellison");
	err_msg("Narayanan Saravanan");
	err_msg("Youngbum Kim - 107387376");
	err_msg("Assignment #2");
	err_msg("----------------------------------------");
	err_msg("a2_cli.c");
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
	char bc_addr[20];

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
	Bind(sockfd, (SA *) &servaddr, sizeof(servaddr));
	
	struct sockaddr_in ss_cli, ss_serv;
	char str[INET_ADDRSTRLEN];

	socklen_t len;
	len = sizeof(ss_cli);
	if (getsockname(sockfd, &ss_cli, &len) < 0){
		err_quit("getsockname error");
	}
	err_msg("IP Client");
	err_msg("IP Address : %s", Inet_ntop(AF_INET, &ss_cli.sin_addr, str, sizeof(str)));
	err_msg("Ephemeral port number assigned : %d", ss_cli.sin_port);

	err_msg("----------------------------------------");
	err_msg("Connect a socket");
	port[4] = '\0';
	
	servaddr.sin_port = htons(atoi(port));

	Connect(sockfd, (SA *) &servaddr, sizeof(servaddr));

	len = sizeof(ss_serv);
	if (getpeername(sockfd, &ss_serv, &len) < 0){
		err_quit("getpeername error");
	}
	err_msg("IP Server");
	err_msg("IP Address : %s", Inet_ntop(AF_INET, &ss_serv.sin_addr, str, sizeof(str)));
	err_msg("Well-known port number : %d", ss_serv.sin_port);
		
	////////
	send_cli(stdin, sockfd, (SA *) &servaddr, sizeof(servaddr));

	exit(0);
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

// Check if the two addr is in the local network
int isLocalNetwork(char *cli_addr, char *serv_addr, char *mask_addr){
	char addr1[4], addr2[4], addr3[4], addr4[4];
	char addr5[4], addr6[4], addr7[4], addr8[4];
	char addr9[4], addr10[4], addr11[4], addr12[4];
	char *network_addr_cli = (char *) calloc(16, sizeof(char)); 
	char *network_addr_serv = (char *) calloc(16, sizeof(char)); 
	char cli_ip_addr[INET_ADDRSTRLEN];
	char serv_ip_addr[INET_ADDRSTRLEN];

	strcpy(cli_ip_addr, serv_addr);
	strcpy(serv_ip_addr, serv_addr);

	strcpy(addr1, strtok(cli_ip_addr, "."));
	strcpy(addr2, strtok(NULL, "."));
	strcpy(addr3, strtok(NULL, "."));
	strcpy(addr4, strtok(NULL, "."));

	strcpy(addr5, strtok(mask_addr, "."));
	strcpy(addr6, strtok(NULL, "."));
	strcpy(addr7, strtok(NULL, "."));
	strcpy(addr8, strtok(NULL, "."));

	strcpy(addr9, strtok(serv_ip_addr, "."));
	strcpy(addr10, strtok(NULL, "."));
	strcpy(addr11, strtok(NULL, "."));
	strcpy(addr12, strtok(NULL, "."));

	snprintf(network_addr_cli, sizeof(network_addr_cli), "%d.%d.%d.%d", atoi(addr1) & atoi(addr5), atoi(addr2) & atoi(addr6), atoi(addr3) & atoi(addr7), atoi(addr4) & atoi(addr8));
	snprintf(network_addr_serv, sizeof(network_addr_serv), "%d.%d.%d.%d", atoi(addr9) & atoi(addr5), atoi(addr10) & atoi(addr6), atoi(addr11) & atoi(addr7), atoi(addr12) & atoi(addr8));

	return strcmp(network_addr_cli, network_addr_serv);
}

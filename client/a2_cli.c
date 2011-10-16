#include "unp.h"
#include "code/unpifiplus.h"
#include "code/get_ifi_info_plus.c"
#include <stdio.h>
#include <string.h>

int *mask(char*, char*, char*);

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
	char serv_ip_addr[20];
	char cli_ip_addr[20];
	char local_cli_ip_addr[20];
	char port[20];
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
		err_msg("|1|Server address: %s", serv_ip_addr);
		
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
				
				err_msg("|2|Server address: %s", serv_ip_addr);
				
				if(!isLocal){
					char network_mask[20];
					strcpy(network_mask, Sock_ntop_host(ifi->ifi_ntmaddr, sizeof(*sa)));
					
					//printf("Network address for client: %s", mask(cli_ip_addr, network_mask));
					err_msg("|2.5|Server address: %s", serv_ip_addr);
					if (mask(cli_ip_addr, serv_ip_addr, network_mask) == 0){
						err_msg("|2.6|Server address: %s", serv_ip_addr);
						strcpy(local_cli_ip_addr, Sock_ntop_host(sa, sizeof(*sa)));
						isLocal = 1;
					}
				}
				
				err_msg("|3|Server address: %s", serv_ip_addr);

				if(!isSameHost){
					if(strcmp(Sock_ntop_host(sa, sizeof(*sa)), serv_ip_addr) == 0) isSameHost = 1;
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
	}
	
	
	if(isLocal){
		err_msg("Server is local");
		strcpy(cli_ip_addr, local_cli_ip_addr);
	}
	
	err_msg("Client address: %s", cli_ip_addr);
	err_msg("Server address: %s", serv_ip_addr);

	int					sockfd;
	struct sockaddr_in	servaddr;

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

	sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

	dg_cli(stdin, sockfd, (SA *) &servaddr, sizeof(servaddr));

	exit(0);
}

int *mask(char *ip_addr, char *serv_addr, char *mask_addr){
	
	char addr1[4], addr2[4], addr3[4], addr4[4];
	char addr5[4], addr6[4], addr7[4], addr8[4];
	char addr9[4], addr10[4], addr11[4], addr12[4];
	char *network_addr_cli = (char *) calloc(16, sizeof(char)); 
	char *network_addr_serv = (char *) calloc(16, sizeof(char)); 
	char serv_ip_addr[20];

	strcpy(serv_ip_addr, serv_addr);

	strcpy(addr1, strtok(ip_addr, "."));
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

	//printf("\n");
	//printf("ip = %d.%d.%d.%d\n", atoi(addr1), atoi(addr2), atoi(addr3), atoi(addr4));
	//printf("network mask = %d.%d.%d.%d\n", atoi(addr5), atoi(addr6), atoi(addr7), atoi(addr8));
	
	snprintf(network_addr_cli, sizeof(network_addr_cli), "%d.%d.%d.%d", atoi(addr1) & atoi(addr5), atoi(addr2) & atoi(addr6), atoi(addr3) & atoi(addr7), atoi(addr4) & atoi(addr8));
	snprintf(network_addr_serv, sizeof(network_addr_serv), "%d.%d.%d.%d", atoi(addr9) & atoi(addr5), atoi(addr10) & atoi(addr6), atoi(addr11) & atoi(addr7), atoi(addr12) & atoi(addr8));
//	printf("network address_cli = %d.%d.%d.%d\n", atoi(addr1) & atoi(addr5), atoi(addr2) & atoi(addr6), atoi(addr3) & atoi(addr7), atoi(addr4) & atoi(addr8));
//	printf("network address_serv = %d.%d.%d.%d\n", atoi(addr9) & atoi(addr5), atoi(addr10) & atoi(addr6), atoi(addr11) & atoi(addr7), atoi(addr12) & atoi(addr8));

	return strcmp(network_addr_cli, network_addr_serv);
}

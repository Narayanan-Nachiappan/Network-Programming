#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

//Function isLocal
//char *cli_addr - Client IP address
//char *serv_addr - Server IP address
//char *mask_addr - Netmask

int isLocal(struct sockaddr *cli_addr, struct sockaddr  *serv_addr, struct sockaddr *netmask){
	
	//printf("isLocal - Client: %s, Server: %s, Netmask: %s\n",Sock_ntop(cli_addr, sizeof(*cli_addr)), Sock_ntop(serv_addr, sizeof(*serv_addr)), Sock_ntop(netmask, sizeof(*netmask)));
	int i = 0;
	char cli_subnet[14], serv_subnet[14];
	
	for(i = 0; i < 14; i++)
	{
		cli_subnet[i] = cli_addr->sa_data[i] & netmask->sa_data[i];
		serv_subnet[i] = serv_addr->sa_data[i] & netmask->sa_data[i];
	}
	
	//printf("Client Subnet: %s, Server Subnet: %s\n", cli_subnet, serv_addr);
	return strncmp(cli_subnet, serv_subnet, 14);
}

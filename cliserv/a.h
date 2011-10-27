#include	<string.h>

#define HD_INIT_CLI		1 // The client sends a datagram to the server giving the filename for the transfer.
#define HD_INIT_SERV	2 // The server sends a datagram to the client giving the port number for connection socket.
#define HD_ACK			3

static struct message {
  uint32_t	seq;	/* sequence # */
  uint32_t	ts;
  uint32_t	type;
  char	data[MAXLINE];		/* timestamp when sent */
  
} send_msg, recv_msg;

struct message messageFactory(int protocol, char *msg){
	printf("hihi");
	fflush(stdout);

	struct message packet;
	
	switch(protocol){
	case HD_INIT_CLI:
		printf("hihi2");
		
		packet.type = protocol;
		strcpy(packet.data, msg);
		break;
	case 2:
		break;
	}
	return packet;
}

int isTypeOf(struct message msg, int protocol){
	if (msg.type == protocol) return 1;
	else return -1;
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

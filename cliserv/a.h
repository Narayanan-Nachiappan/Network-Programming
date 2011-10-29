#include	<string.h>
#include "unprtt.h"

#define HD_INIT_CLI		1 // The client sends a datagram to the server giving the filename for the transfer.
#define HD_INIT_SERV	2 // The server sends a datagram to the client giving the port number for connection socket.
#define HD_INIT_ACK		3 // The client sends a acknolodge to the server

static struct message {
  uint32_t	seq;	/* sequence # */
  uint32_t	ts;
  uint32_t wind_size;
  uint32_t	type;
  char	data[MAXLINE];		/* timestamp when sent */
  
} send_msg, recv_msg;

struct message messageFactory(int protocol, char *msg){
	struct message packet;
	packet.type = protocol;
	strcpy(packet.data, msg);
	return packet;
}

int getIntMsg(struct message msg){
	return atoi(msg.data);
}

void printMessage(struct message msg){
	err_msg("----------------------------------------");
	err_msg("Packet received");
	err_msg("Time Stamp: %d", msg.ts);
	err_msg("Sequence#: %d", msg.seq);
	err_msg("Type: %d", msg.type);
	err_msg("Message: %s", msg.data);
}

int isTypeOf(struct message msg, int protocol){
	if (msg.type == protocol) return 1;
	else return -1;
}

void dg_client( int sockfd,  SA *pservaddr, socklen_t servlen, uint32_t windSize){
	int n;
	socklen_t len;
	int i=1;
	char	sendline[MAXLINE], recvline[MAXLINE ],outstr[MAXLINE + 1];
	static struct rtt_info   rttinfo;
	static int	rttinit = 0;
		len=servlen;
		fprintf(stderr,"\n window size %d ",windSize);
		
		//n = read(sockfd, (char*)&recv_msg, MAXLINE);
		
		//n = recvfrom(sockfd, (char*)&recv_msg, MAXLINE, 0, pservaddr, &len);
		//n = recvfrom(sockfd, (char*)&recv_msg, MAXLINE, 0, NULL, NULL);
		n = recv(sockfd, (char*)&recv_msg, MAXLINE, 0);
	while (n>0) {
		printf("hohoho");
		recvline[n] = 0;	/* null terminate */
		sprintf(outstr,"\nrecv datagram %d from server\n",recv_msg.seq);
		Fputs(outstr,stdout);
		fflush(stdout);
		fprintf(stderr,"\n %s",recv_msg.data);

		//fprintf(stderr,"\n %d",recv_msg.seq);

	//	if((recv_msg.seq%2)==0){
		if (rttinit == 0) {
		rtt_init(&rttinfo);		/* first time we're called */
		rttinit = 1;
		rtt_d_flag = 1;
	}
	rtt_newpack(&rttinfo);		/* initialize for this packet */
	send_msg.ts = rtt_ts(&rttinfo);


		sprintf(outstr,"\nsending ack for received datagram %d \n",recv_msg.seq);
		Fputs(outstr,stdout);
		send_msg.seq=recv_msg.seq;
		strcpy(send_msg.data,"ACK");
		send_msg.wind_size=windSize;

		fflush(stdout);
		Writen(sockfd, (char *)&send_msg, n);
	//	}

		n = Recvfrom(sockfd, (struct message *)&recv_msg, MAXLINE, 0, pservaddr, &len);
		
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

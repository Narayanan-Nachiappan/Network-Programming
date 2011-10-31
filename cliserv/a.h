#include	<string.h>
#include "unprtt.h"

#define HD_INIT_CLI		1 // The client sends a datagram to the server giving the filename for the transfer.
#define HD_INIT_SERV	2 // The server sends a datagram to the client giving the port number for connection socket.
#define HD_FILE			3 // The server sends a packet to the client .
#define HD_FILE_ACK		4 // The client sends a acknolodge to the server for the packet
#define HD_INIT_ACK		5 // The client sends a acknolodge to the server for the init connection
#define HD_EOF_FILE		6 // The server indicates client the EOF

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

struct queueNode{
	char data[MAXLINE];
	struct queueNode *nextPtr;
} *headPtr, *tailPtr ;

static struct message {
  uint32_t	seq;	/* sequence # */
  uint32_t	ts;
  uint32_t wind_size;
  uint32_t	type;		/* timestamp when sent */
  char	data[496];
  
} send_msg, recv_msg;

void printQueue(){
	err_msg("----------------------------------------");
	err_msg("* Printing Queue");
	struct queueNode *tempPtr = headPtr;
	while(tempPtr != NULL){
		err_msg("%s", tempPtr->data);
		tempPtr = tempPtr->nextPtr;
	}
}

void enqueue(char *dataLine){
	Pthread_mutex_lock(&mutex);
	
	struct queueNode *newPtr;
	newPtr = (struct queueNode*)malloc( sizeof(struct queueNode) );
	
	if(newPtr != NULL){
		strcpy(newPtr->data, dataLine);
		newPtr->nextPtr = NULL;
		if(headPtr == NULL) {
			headPtr = newPtr;
		} else {
			tailPtr->nextPtr = newPtr;
		}
		tailPtr = newPtr;
	} else {
		err_msg("No memory available");
	}
	
	Pthread_mutex_unlock(&mutex);
}

void dequeue(){
	err_msg("----------------------------------------");
	err_msg("* Dequeue");

	Pthread_mutex_lock(&mutex);
	
	char value[MAXLINE];
	struct queueNode *tempPtr;
	strcpy(value, (char *)(headPtr->data));
	tempPtr = headPtr;
	headPtr = headPtr->nextPtr;
	if(headPtr == NULL){
		tailPtr = NULL;
	}
	free(tempPtr);

	Pthread_mutex_unlock(&mutex);

	err_msg("Message: %s", value);
}

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
	err_msg("Packet received (%d Byte)", sizeof(msg));
	err_msg("Time Stamp: %d", msg.ts);
	err_msg("Sequence#: %d", msg.seq);
	err_msg("Type: %d", msg.type);
	err_msg("Message: %s", msg.data);
}

void dg_client( int sockfd,  SA *pservaddr, socklen_t servlen, uint32_t windSize){
	int n;
	socklen_t len;
	int i=1;
	char	sendline[MAXLINE], recvline[MAXLINE ],outstr[MAXLINE + 1];
	static struct rtt_info   rttinfo;
	static int	rttinit = 0;
		len=servlen;
		fprintf(stderr,"Window size: %d\n",windSize);
		n = recv(sockfd, (char*)&recv_msg, MAXLINE, 0);
	while (n>0) {
		enqueue(recv_msg.data);
		//printMessage(recv_msg);
		//printQueue();

		if (rttinit == 0) {
			rtt_init(&rttinfo);		/* first time we're called */
			rttinit = 1;
			rtt_d_flag = 1;
		}
		rtt_newpack(&rttinfo);		/* initialize for this packet */
		send_msg.ts = rtt_ts(&rttinfo);

		sprintf(outstr,"* sending ack for received datagram %d \n",recv_msg.seq);
		Fputs(outstr,stdout);
		send_msg.seq=recv_msg.seq;
		send_msg.type = HD_FILE_ACK;
		strcpy(send_msg.data,"ACK");
		send_msg.wind_size=windSize;

		fflush(stdout);
		Writen(sockfd, (char *)&send_msg, n);

		if(recv_msg.type == HD_EOF_FILE){
			err_msg("EOF");
			err_msg("Client in TIME_WAIT status");
			break;
		} else if(recv_msg.type == HD_FILE){
			n = Recvfrom(sockfd, (struct message *)&recv_msg, MAXLINE, 0, pservaddr, &len);	
		} else if(recv_msg.type == 0){
			err_msg("Wrong protocol");
			n = Recvfrom(sockfd, (struct message *)&recv_msg, MAXLINE, 0, pservaddr, &len);	
		}
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

void *printBuffer(){ // thread that dequeues and prints recv_buffer
	
	while(1){
		//printf("\nThread Test\n");
		while(headPtr != NULL){
			dequeue();
		}// else {
			//printf("Thread Test - queue is empty ");
		//}
		//sleep(10);
	}

}

int expo(){
	//int result = -1 * ln(sran(seed)) * miu;
	//return result;
	return 0;
}

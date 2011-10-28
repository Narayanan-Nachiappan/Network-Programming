#include	"unp.h"
#include "struct.c"
#include	"unprtt.h"


void dg_client( int sockfd,  SA *pservaddr, socklen_t servlen, uint32_t windSize)
{

	int n;
	 socklen_t len;
	int i=1;
	char	sendline[MAXLINE], recvline[MAXLINE ],outstr[MAXLINE + 1];
	static struct rtt_info   rttinfo;
static int	rttinit = 0;
		len=servlen;
		fprintf(stderr,"\n window size %d ",windSize);
		n = Recvfrom(sockfd, (char*)&recv_msg, MAXLINE, 0, pservaddr, &len);
	while (n>0) {
		
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
		Sendto(sockfd, (char *)&send_msg, n, 0, pservaddr, servlen);
	//	}

		n = Recvfrom(sockfd, (struct message *)&recv_msg, MAXLINE, 0, pservaddr, &len);
		
	}
}






int
main(int argc, char **argv)
{
	int					sockfd;
	struct sockaddr_in	servaddr;
	int	n;
	uint32_t wind_size=10;
	char	sendline[MAXLINE], recvline[MAXLINE + 1];
	const SA *pservaddr;
	if (argc != 2)
		err_quit("usage: udpcli <IPaddress>");

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(2345);
	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

	sockfd = Socket(AF_INET, SOCK_DGRAM, 0);
	sprintf(sendline,"Init");
	fflush(stdout);
	Fputs("Client : Initiating the connection\n",stdout);
	fflush(stdout);
	pservaddr= (SA *) &servaddr;
	Sendto(sockfd, sendline, strlen(sendline), 0,  pservaddr, sizeof(servaddr));

		fflush(stdout);
	n = Recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);
		fflush(stdout);

	recvline[n] = 0;	/* null terminate */
	fflush(stdout);
	Fputs("Reply from server for init\n",stdout);
	Fputs("####", stdout);
	Fputs(recvline, stdout);
	fflush(stdout);
//----------------------------------------Method for cli---------------------
	dg_client( sockfd, (SA *) &servaddr, sizeof(servaddr),wind_size);

	exit(0);
}

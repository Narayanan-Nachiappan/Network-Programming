#include	"unp.h"


	static struct hdr {
  uint32_t	seq;	/* sequence # */
  uint32_t	ts;
  char	*data;		/* timestamp when sent */
  
} sendhdr, recvhdr;


void dg_client( int sockfd,  SA *pservaddr, socklen_t servlen)
{

	int n;
	 socklen_t len;
	int i=1;
	char	sendline[MAXLINE], recvline[MAXLINE ],outstr[MAXLINE + 1];
	
		len=servlen;
		n = Recvfrom(sockfd, (char*)&recvhdr, MAXLINE, 0, pservaddr, &len);
	while (n>0) {
		
		recvline[n] = 0;	/* null terminate */
		sprintf(outstr,"\nrecv datagram %d from server\n",i);
		Fputs(outstr,stdout);
		fflush(stdout);
		fprintf(stderr,"\n %s",recvhdr.data);

		fprintf(stderr,"\n %d",recvhdr.seq);

		if((recvhdr.seq%2)==0){

		sprintf(outstr,"\nsending ack for received datagram %d , %d\n",recvhdr.seq-1,recvhdr.seq);
		Fputs(outstr,stdout);
		
		fflush(stdout);
		Sendto(sockfd, (char *)&recvhdr, n, 0, pservaddr, servlen);
		}

		n = Recvfrom(sockfd, (char *)&recvhdr, MAXLINE, 0, pservaddr, &len);
		
	}
}






int
main(int argc, char **argv)
{
	int					sockfd;
	struct sockaddr_in	servaddr;
	int	n;
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

	dg_client( sockfd, (SA *) &servaddr, sizeof(servaddr));

	exit(0);
}

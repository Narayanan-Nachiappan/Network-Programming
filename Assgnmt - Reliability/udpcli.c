#include	"unp.h"




void dg_client( int sockfd, const SA *pservaddr, socklen_t servlen)
{
	static struct msghdr	msgsend, msgrecv;	/* assumed init to 0 */
	static struct hdr {
		uint32_t	seq;	/* sequence # */
		uint32_t	ts;		/* timestamp when sent */
	} sendhdr, recvhdr;
	
	int i=1;
	char	sendline[MAXLINE], recvline[MAXLINE + 1],outstr[MAXLINE + 1];	
	ssize_t			n;
	struct iovec	iovsend[2], iovrecv[2];
	msgsend.msg_name = pservaddr;
	msgsend.msg_namelen = servlen;
	msgsend.msg_iov = iovsend;
	msgsend.msg_iovlen = 2;
	iovsend[0].iov_base = &sendhdr;
	iovsend[0].iov_len = sizeof(struct hdr);
	iovsend[1].iov_base = "ACK";
	iovsend[1].iov_len = sizeof("ACK");

	msgrecv.msg_name = NULL;
	msgrecv.msg_namelen = 0;
	msgrecv.msg_iov = iovrecv;
	msgrecv.msg_iovlen = 2;
	iovrecv[0].iov_base = &recvhdr;
	iovrecv[0].iov_len = sizeof(struct hdr);
	iovrecv[1].iov_base = "";
	iovrecv[1].iov_len = 0;

	n = Recvmsg(sockfd, &msgrecv, 0);

		
	while (n>0) {
		recvline[n] = 0;	/* null terminate */
		sprintf(outstr,"\nrecv datagram %d from server\n",recvhdr.seq);
		Fputs(outstr,stdout);
		fflush(stdout);
		Fputs(iovrecv[1].iov_base,stdout);
		fflush(stdout);
		sprintf(outstr,"\nsending ack for received datagram %d\n",recvhdr.seq);
		Fputs(outstr,stdout);
		fflush(stdout);
		sendhdr.seq=recvhdr.seq;
		Sendmsg(sockfd, &msgsend, 0);
		i++;
		n = Recvmsg(sockfd, &msgrecv, 0);
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

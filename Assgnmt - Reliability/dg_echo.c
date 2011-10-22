#include	"unp.h"




ssize_t	Dg_send_recv(int, const void *, size_t, void *, size_t,
				   const SA *, socklen_t);


void
dg_echo(File * fp,int sockfd, SA *pcliaddr, socklen_t clilen)
{
	socklen_t	len;
	char		mesg[MAXLINE];
	ssize_t	n;
	char	sendline[MAXLINE], recvline[MAXLINE + 1],prin[MAXLINE + 1];
	while (Fgets(sendline, MAXLINE, fp) != NULL) {

		n = Dg_send_recv(sockfd, sendline, strlen(sendline),
						 recvline, MAXLINE, pcliaddr, clilen);
		
		recvline[n] = 0;	/* null terminate */
		sprintf(prin,"Received ack for datagram %d",recvline);
		Fputs(recvline, stdout);
	}
}

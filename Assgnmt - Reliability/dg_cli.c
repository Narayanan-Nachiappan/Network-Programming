#include	"unp.h"




void dg_cli( int sockfd, const SA *pservaddr, socklen_t servlen)
{
	int n;
	char	sendline[MAXLINE], recvline[MAXLINE + 1];
	

		n = Recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);
	while (n>0) {
		recvline[n] = 0;	/* null terminate */
		Fputs("Datagram from server",stdout);
		Fputs(recvline,stdout);
		Fputs("Sending ACK for received datagram",stdout);
		Sendto(sockfd, recvline, strlen(recvline), 0, pservaddr, servlen);
		n = Recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);
		
	}
}

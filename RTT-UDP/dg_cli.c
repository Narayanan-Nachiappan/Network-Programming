#include	"unp.h"

ssize_t	Dg_send_recv(int, const void *, size_t, void *, size_t,
				   const SA *, socklen_t);

void
dg_cli(FILE *fp, int sockfd, const SA *pservaddr, socklen_t servlen)
{
	int n;
	char	sendline[MAXLINE], recvline[MAXLINE + 1];

	while (Fgets(sendline, MAXLINE, fp) != NULL) {

		n = Dg_send_recv(sockfd, sendline, strlen(sendline),
						 recvline, MAXLINE, pservaddr, servlen);

		recvline[n] = 0;	/* null terminate */
		printf("Ack from server for packet %d",n );
	}
}

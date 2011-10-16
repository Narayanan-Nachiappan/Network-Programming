#include	"unp.h"

ssize_t	Dg_send_recv(int, const void *, size_t, void *, size_t,
				   const SA *, socklen_t);

void
dg_cli(FILE *fp, int sockfd, const SA *pservaddr, socklen_t servlen)
{
	ssize_t	n;
	char	sendline[MAXLINE], recvline[MAXLINE + 1];
	int i;
	Fgets(sendline, MAXLINE, fp);
for (i = 0; i < 15; i++) {
	

		n = Dg_send_recv(sockfd, "data to be send", strlen("data to be send"),
						 recvline, MAXLINE, pservaddr, servlen);

	
}
}
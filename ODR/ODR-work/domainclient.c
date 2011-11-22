#include	"unp.h"
#include "constants.h"
#define CLI_PATH "GROUP15_12092_VI09Ig"
void
dg_cli_time(FILE *fp, int sockfd, const SA *pservaddr, socklen_t servlen)
{
	int	n;
	char	sendline[MAXLINE], recvline[MAXLINE + 1];

	//while (Fgets(sendline, MAXLINE, fp) != NULL) {
		strcpy(sendline,"Time request from the client");
		Sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen);

		n = Recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);

		 //recvline[n] = 0;	/* null terminate */
		fprintf(stdout,"%s",recvline);
	//}
}

int
main(int argc, char **argv)
{
	int					sockfd;
	struct sockaddr_un	cliaddr, servaddr;

	sockfd = Socket(AF_LOCAL, SOCK_DGRAM, 0);

	bzero(&cliaddr, sizeof(cliaddr));		/* bind an address for us */
	cliaddr.sun_family = AF_LOCAL;
	strcpy(cliaddr.sun_path, tmpnam(NULL));
	unlink( tmpnam(NULL));
	Bind(sockfd, (SA *) &cliaddr, sizeof(cliaddr));

	bzero(&servaddr, sizeof(servaddr));	/* fill in server's address */
	servaddr.sun_family = AF_LOCAL;
	strcpy(servaddr.sun_path, CLI_PATH);

	dg_cli_time(stdin, sockfd, (SA *) &servaddr, sizeof(servaddr));

	exit(0);
}

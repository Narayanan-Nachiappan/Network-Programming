#include	"unp.h"
#include <time.h>
#include "constants.h"

void
dg_echo_time(int sockfd, SA *pcliaddr, socklen_t clilen)
{
	int			n;
	socklen_t	len;
	char		mesg[MAXLINE];
		time_t ticks;
		for(;;){
		len = clilen;
		n = Recvfrom(sockfd, mesg, MAXLINE, 0, pcliaddr, &len);
		ticks = time(NULL);
        snprintf(mesg, sizeof(mesg), "%.24s\r\n", ctime(&ticks));
		fprintf(stdout,"Sending Time using domain socket");
		fprintf(stdout,"%s",mesg);
		Sendto(sockfd, mesg, n, 0, pcliaddr, clilen);
		}
}

int
main(int argc, char **argv)
{
	int					sockfd;
	struct sockaddr_un	servaddr, cliaddr;

	sockfd = Socket(AF_LOCAL, SOCK_DGRAM, 0);

	unlink(SERVER_UNIX_DG_PATH);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sun_family = AF_LOCAL;
	strcpy(servaddr.sun_path, SERVER_UNIX_DG_PATH);

	Bind(sockfd, (SA *) &servaddr, sizeof(servaddr));

	dg_echo_time(sockfd, (SA *) &cliaddr, sizeof(cliaddr));
}

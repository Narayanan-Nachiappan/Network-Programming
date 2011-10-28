#include	"unp.h"
#include "struct.c"


ssize_t	Dg_send_recv(int, const void *, size_t, void *, size_t,
				   const SA *, socklen_t);


void dg_echofun(FILE * fp,int sockfd, const SA *pcliaddr, socklen_t clilen)
{
	int i=1;
	socklen_t	len;
	char		mesg[MAXLINE];
	ssize_t	n;
	char	sendline[MAXLINE], recvline[MAXLINE + 1],prin[MAXLINE + 1];
	fprintf(stderr, "Slow start#### Init wind size %4d ", i);
	while (Fgets(sendline, MAXLINE, fp) != NULL) {
		//Fputs(sendline,stdout);
		
		n = Dg_send_recv(sockfd, sendline, strlen(sendline),
						 recvline, MAXLINE, pcliaddr, clilen);
		
		recvline[n] = 0;	/* null terminate */
		
		
		Fputs(prin,stdout);
		fflush(stdout);
		//Fputs(recvline,stdout);
		//fflush(stdout);
	}

}



int
main(int argc, char **argv)
{
	int					sockfd;
	struct sockaddr_in	servaddr, cliaddr;
	int			n;
	socklen_t len;
	char		mesg[MAXLINE];
	sockfd = Socket(AF_INET, SOCK_DGRAM, 0);
	SA *pcliaddr;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(2345);

	Bind(sockfd, (SA *) &servaddr, sizeof(servaddr));
	len=sizeof(cliaddr);

	pcliaddr= (SA *)&cliaddr;
	
	n = Recvfrom(sockfd, mesg, MAXLINE, 0,  pcliaddr, &len);
	
	Fputs("Init req from client\n",stdout);
	Fputs("####",stdout);
	Fputs(mesg,stdout);
	sprintf(mesg,"Init reply from server");
	Sendto(sockfd, mesg, sizeof(mesg), 0, pcliaddr, len);
	Fputs("\nStarting File transfer\n",stdout);

	//------------------Method for server-------------------------------------------
	dg_echofun(stdin,sockfd, (SA *) &cliaddr, len);
	return 0;
}

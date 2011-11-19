#include "constants.h"
void
dg_send_recv_cli( int sockfd, char * canip,int port,char *message,int flag)

{
	//fprintf(stderr,"%d",sockfd);
	int	n;
	char	sendline[MAXLINE], recvline[MAXLINE + 1];
struct  sockaddr_un servaddr;
servaddr.sun_family = AF_LOCAL;
	strcpy(servaddr.sun_path, SERVER_UNIX_DG_PATH);

	//while (Fgets(sendline, MAXLINE, fp) != NULL) {
		strcpy(sendline,"Time request from the client");
		Sendto(sockfd, sendline, strlen(sendline), 0, (struct sockaddr *)&servaddr, sizeof(struct sockaddr));

		n = Recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);

		 //recvline[n] = 0;	/* null terminate */
		fprintf(stdout,"%s",recvline);
	//}
}


void
dg_send_recv_srv(int sockfd,char *canip, char *msg,int port,int flag)
{
	int			n;
//	socklen_t	len;
	struct sockaddr_un cliaddr;
	char		mesg[MAXLINE];
		time_t ticks;
		
		socklen_t len=sizeof(struct sockaddr);
		n = Recvfrom(sockfd, mesg, MAXLINE, 0, (struct sockaddr *)&cliaddr, &len);
		ticks = time(NULL);
        snprintf(mesg, sizeof(mesg), "%.24s\r\n", ctime(&ticks));
		fprintf(stdout,"Sending Time using domain socket");
		fprintf(stdout,"%s",mesg);
		Sendto(sockfd, mesg, n, 0,  (struct sockaddr *)&cliaddr, len);
		
}
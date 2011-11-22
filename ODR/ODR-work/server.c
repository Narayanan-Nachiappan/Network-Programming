#include	"unp.h"
#include <time.h>
#include "sendrecv.c"



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
char * message,time[MAXLINE];
char address[16];
int port;
//	dg_send_recv_srv(sockfd,"128.9.1.1","Message",444444,1);
msg_recv(sockfd,message,address,&port,2);
time_t ticks;
snprintf(time, sizeof(time), "%.24s", ctime(&ticks));
err_msg("%s",address);
err_msg("%d",port);
msg_send(sockfd,address,port,time,0);
}
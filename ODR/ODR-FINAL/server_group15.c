#include	"unp.h"
#include <time.h>
#include "sendrecv.c"

#define SIZE 1024

int main(int argc, char **argv){
	err_msg("CSE 533 : Network Programming");
	err_msg("Amelia Ellison - 107838108");
	err_msg("Narayanan Nachiappan - 107996031");
	err_msg("Youngbum Kim - 107387376");
	err_msg("Assignment #3");
	err_msg("----------------------------------------");
	err_msg("server.c");
	err_msg("----------------------------------------");

	int					sockfd;
	struct sockaddr_un	servaddr, cliaddr;
	char hostName[SIZE];

	gethostname(hostName, sizeof(hostName));

	err_msg("Host Name: %s", hostName);
	err_msg("----------------------------------------");

	sockfd = Socket(AF_LOCAL, SOCK_DGRAM, 0);

	unlink(SERVER_UNIX_DG_PATH);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sun_family = AF_LOCAL;
	strcpy(servaddr.sun_path, SERVER_UNIX_DG_PATH);

	Bind(sockfd, (SA *) &servaddr, sizeof(servaddr));
	
	char * message,time[MAXLINE];
	char address[16];
	int port;
	struct hostent *hptr;
	struct in_addr **pptr;
	struct sockaddr_in tempaddr;
	bzero(&tempaddr, sizeof(tempaddr));
	tempaddr.sin_family = AF_INET;

	while(1){

		msg_recv(sockfd,message,address,&port);
		time_t ticks;
		snprintf(time, sizeof(time), "%.24s", ctime(&ticks));
		
		if (inet_pton(AF_INET, address, &tempaddr.sin_addr) <= 0){
			err_quit("inet_pton error for %s", address);
		}
		pptr = (struct in_addr**) &tempaddr.sin_addr;
		hptr = gethostbyaddr(pptr, sizeof pptr, AF_INET);
		err_msg("server at node %s responding to request from %s", hostName, hptr->h_name);

		err_msg("%s",address);
		err_msg("%d",port);
		msg_send(sockfd,address,port,time,0);

	}

}
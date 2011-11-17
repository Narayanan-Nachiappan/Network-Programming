#include "unp.h"
#include "sendrecv.c"
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

int main(int argc, char **argv){
	err_msg("CSE 533 : Network Programming");
	err_msg("Amelia Ellison - 107838108");
	err_msg("Narayanan Nachiappan - 107996031");
	err_msg("Youngbum Kim - 107387376");
	err_msg("Assignment #3");
	err_msg("----------------------------------------");
	err_msg("client.c");
	err_msg("----------------------------------------");

	int	sockfd;
	int	tempfd;
	char buf[24] = "GROUP15XXXXXX";

	struct sockaddr_un	cliaddr, servaddr;

	sockfd = Socket(AF_LOCAL, SOCK_DGRAM, 0);

	bzero(&cliaddr, sizeof(cliaddr));		/* bind an address for us */
	cliaddr.sun_family = AF_LOCAL;
	
	tempfd = mkstemp(buf);
	strcpy(cliaddr.sun_path, buf);
	
	err_msg("Temporary File name created by mkstemp()");
	err_msg(buf);
	err_msg("----------------------------------------");

	bind(sockfd, (SA *) &cliaddr, sizeof(cliaddr));

	bzero(&servaddr, sizeof(servaddr));	/* fill in server's address */
	servaddr.sun_family = AF_LOCAL;
	strcpy(servaddr.sun_path, UNIXDG_PATH);

	int menu = 0;
	printf("Choose the vm node (1 to 10): ");

	do{
		scanf("%d", &menu);
		fflush(stdin);

		err_msg("client at node vm i1 sending request to server at vm i2");
		msg_send(sockfd, (SA *) &servaddr, 9999, "O", 0);
		
		char message[SIZE];
		int port;
		msg_recv(sockfd, message, (SA *) &servaddr, &port);

	} while(menu > 0);

	unlink(buf);

	exit(0);
}

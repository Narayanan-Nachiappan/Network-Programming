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
	char vmAddr[INET_ADDRSTRLEN];
	char hostName[SIZE];
	struct sockaddr_un	cliaddr, servaddr;

	gethostname(hostName, sizeof(hostName));

	err_msg("Host Name: %s", hostName);
	err_msg("----------------------------------------");

	sockfd = Socket(AF_LOCAL, SOCK_DGRAM, 0);

	bzero(&cliaddr, sizeof(cliaddr));		/* bind an address for us */
	cliaddr.sun_family = AF_LOCAL;
	
	tempfd = mkstemp(buf);
	strcpy(cliaddr.sun_path, buf);
	
	err_msg("Temporary File name created by mkstemp()");
	err_msg(buf);
	err_msg("----------------------------------------");
	unlink(buf);
	Bind(sockfd, (SA *) &cliaddr, sizeof(cliaddr));

	bzero(&servaddr, sizeof(servaddr));	/* fill in server's address */
	servaddr.sun_family = AF_LOCAL;
	strcpy(servaddr.sun_path, SERVER_UNIX_DG_PATH);

	struct hostent *hptr;
	struct in_addr **pptr;
	char *message="TIME_REQUEST";
	int menu = 0;
	

	do{
		printf("Choose the vm node (1 to 10): ");
		scanf("%d", &menu);
		fflush(stdin);

		if(menu < 0 || menu > 10){
			err_msg("Invalid vm number. Exit");
			break;
		}

		char vmName[4];
		sprintf(vmName, "vm%d", menu);
		hptr = gethostbyname(vmName);
		pptr = hptr->h_addr_list;
		Inet_ntop(hptr->h_addrtype, *pptr, vmAddr, sizeof(vmAddr));

		err_msg("%s Address: %s", vmName, vmAddr);
		err_msg("client at node %s sending request to server at %s", hostName, vmName);
	
	//	dg_send_recv_cli( sockfd,vmAddr,444444,"Message",1);
	msg_send(sockfd,vmAddr,SERVER_UNIX_PORT,message,0,1);
	msg_recv(sockfd,message,vmAddr,SERVER_UNIX_PORT,1);

	} while(menu > 0);

	unlink(buf);

	exit(0);
}

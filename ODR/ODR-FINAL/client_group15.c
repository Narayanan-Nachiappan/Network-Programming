#include "unp.h"
#include "sendrecv.c"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>
#include "random.c"
#define SIZE 1024

jmp_buf env;
extern void timeout();

int main(int argc, char **argv){
	err_msg("CSE 533 : Network Programming");
	err_msg("Amelia Ellison - 107838108");
	err_msg("Narayanan Nachiappan - 107996031");
	err_msg("Youngbum Kim - 107387376");
	err_msg("Assignment #3");
	err_msg("----------------------------------------");
	err_msg("client_group15.c");
	err_msg("----------------------------------------");

	int	sockfd;
	int	tempfd;
	char buf[24] = "G_";
	char vmAddr[INET_ADDRSTRLEN];
	char hostName[SIZE];
	struct sockaddr_un	cliaddr, servaddr;

	gethostname(hostName, sizeof(hostName)); /* Get Host name for the node */

	err_msg("Host Name: %s", hostName);
	err_msg("----------------------------------------");

	sockfd = Socket(AF_LOCAL, SOCK_DGRAM, 0);

	bzero(&cliaddr, sizeof(cliaddr));
	cliaddr.sun_family = AF_LOCAL;
	
	int random =randomgenerator(); /* Genarate the random number which will be used as port number */
	char temp[14];
	sprintf(temp, "%d_XXXXXX", random);
	strcat(buf, temp);
	tempfd = mkstemp(buf); /* Add 6 random digits string for the sun_path */

	strcpy(cliaddr.sun_path, buf);

	err_msg("Temporary File name created by mkstemp()");
	err_msg(buf);
	err_msg("----------------------------------------");
	unlink(buf); /* remove the file ( the file will be automatically created when it binds ) */
	Bind(sockfd, (SA *) &cliaddr, sizeof(cliaddr)); /* bind an address for us */

	bzero(&servaddr, sizeof(servaddr));	/* fill in server's address */
	servaddr.sun_family = AF_LOCAL;
	strcpy(servaddr.sun_path, ODR_SUNPATH);

	struct hostent *hptr;
	struct in_addr **pptr;
	char *message="TIME_REQUEST";
	int menu = 0;
	int port;
	/* Enters an infinite loop */
	do{
		printf("Choose the vm node (1 to 10): "); /* Get user input for the destination node */
		scanf("%d", &menu);
		fflush(stdin);

		if(menu < 0 || menu > 10){
			err_msg("Invalid vm number. Exit");
			break;
		}

		char vmName[4]; /* Get the ip address and name for the destination node */
		sprintf(vmName, "vm%d", menu);
		hptr = gethostbyname(vmName);
		pptr = (struct in_addr**) hptr->h_addr_list;
		Inet_ntop(hptr->h_addrtype, *pptr, vmAddr, sizeof(vmAddr));

		err_msg("%s Address: %s", vmName, vmAddr);
		err_msg("client at node %s sending request to server at %s %d", hostName, vmName, time(NULL));

		int i; /* Time out mechanism */
		signal(SIGALRM, timeout);
		/* Retry to send only onetime after 5 seconds */
		if (setjmp(env) == 0) {
			alarm(5);
			msg_send(sockfd,vmAddr,SERVER_UNIX_PORT,message,0);
			alarm(0);
		} else {
			msg_send(sockfd,vmAddr,SERVER_UNIX_PORT,message,1);
		}
	msg_recv(sockfd,message,vmAddr,&port);

	} while(menu > 0);

	unlink(buf);

	exit(0);
}

void timeout(int sig){
  int i;
  signal(sig, SIG_IGN);
  printf("Timeout.");
  signal(SIGALRM, timeout);
  longjmp(env, 1);
}

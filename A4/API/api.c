
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>
#include "tour.h"
#include "random.c"
#include "api.h"
#define SIZE 1024

jmp_buf env;
extern void timeout();

int areq (struct sockaddr *IPaddr, socklen_t sockaddrlen, struct hwaddr *HWaddr){
	
	int	sockfd;
	int	tempfd;
	char buf[24] = "GROUP15_";
	char vmAddr[INET_ADDRSTRLEN];
	char hostName[SIZE];
	struct sockaddr_un	cliaddr, servaddr;
	sockfd = Socket(AF_LOCAL, SOCK_STREAM, 0);
	bzero(&cliaddr, sizeof(cliaddr));	
	cliaddr.sun_family = AF_LOCAL;
	int random =randomgenerator();
	char temp[14];
	sprintf(temp, "%d_XXXXXX", random);
	strcat(buf, temp);
	tempfd = mkstemp(buf);
	strcpy(cliaddr.sun_path, buf);
	unlink(buf);
	Bind(sockfd, (SA *) &cliaddr, sizeof(cliaddr));
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sun_family = AF_LOCAL;
	strcpy(servaddr.sun_path, ODR_TOUR_SUNPATH);
	signal(SIGALRM, timeout);
	if(sendto(sockfd, (char *)IPaddr, sockaddrlen, 0, (struct sockaddr*) &servaddr, sizeof(struct sockaddr)) < 0)
	{
		printf("sendto error: %d %s\n", errno, strerror(errno));
		return -1;
	}
	if (setjmp(env) == 0) {
			alarm(10);
			recvfrom(sockfd,(struct hwaddr *)HWaddr,vmAddr,SERVER_UNIX_PORT,1);
			alarm(0);
	}
	else{
		return -1;
	}

	unlink(buf);
	return 1;
}

void timeout(int sig){
  int i;
  signal(sig, SIG_IGN);
  printf("Timeout.");
  signal(SIGALRM, timeout);
  longjmp(env, 1);
}

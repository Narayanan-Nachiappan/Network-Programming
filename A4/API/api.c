
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>
#include "api.h"
#define SIZE 1024
#define CLI_SUN_PATH "g15_api_path"
#define ARP_DG_PATH "g15_arppath"
jmp_buf env;
extern void timeout();

int areq (struct sockaddr *IPaddr, socklen_t sockaddrlen, struct hwaddr *HWaddr){
	
	int	sockfd;
	int	tempfd;
	int yes=1;
	char sendline[100];
	char vmAddr[INET_ADDRSTRLEN];
	char hostName[SIZE];
	struct sockaddr_un	cliaddr, servaddr;
	sockfd = socket(AF_LOCAL, SOCK_STREAM, 0);
	if(sockfd<0){
		err_msg("%s",strerror(errno));
	}
	bzero(&cliaddr, sizeof(cliaddr));	
	cliaddr.sun_family = AF_LOCAL;
	char temp[14];
	strcpy(cliaddr.sun_path, CLI_SUN_PATH);
	unlink(CLI_SUN_PATH);
if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            err_msg("setsockopt");
            return -1;
        }
	if ((bind(sockfd, (SA *) &cliaddr, sizeof(cliaddr)))<0)
	{
		err_msg("bind error %s",strerror(errno));
	}
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sun_family = AF_LOCAL;
	strcpy(servaddr.sun_path, ARP_DG_PATH);
	signal(SIGALRM, timeout);

	sprintf(sendline,"%s,%d,%u,%c",IPaddr->sa_data,HWaddr->sll_ifindex,HWaddr->sll_hatype,HWaddr->sll_halen);
	/*	strcat(sendline,IPaddr->sa_data);
strcat(sendline,",");
	strcat(sendline,itoa(HWaddr->sll_ifindex));
	strcat(sendline,",");
	strcat(sendline,itoa(HWaddr->sll_hatype));
strcat(sendline,",");
	strcat(sendline,itoa(HWaddr->sll_halen));*/
	strcat(sendline,"\0");

 if (connect(sockfd,
             (struct sockaddr *) &(servaddr),
             sizeof(struct sockaddr)) < 0) {
  		printf(" connect error: %d %s\n", errno, strerror(errno));
		return -1;
 }

//	if(sendto(sockfd, "hi", sizeof("hi"), 0, (struct sockaddr*) &servaddr, sizeof(struct sockaddr)) < 0)
//	{
//			printf("sendto error: %d %s\n", errno, strerror(errno));
//		return -1;
//	}
Writen(sockfd, "hi", strlen("hi"));
	if (setjmp(env) == 0) {
			alarm(10);
			recvfrom(sockfd, (struct hwaddr *)HWaddr, MAXLINE, 0, NULL, NULL);
			alarm(0);
	}
	else{
		return -1;
	}

	unlink(CLI_SUN_PATH);
	return 1;
}

void timeout(int sig){
  int i;
  signal(sig, SIG_IGN);
  printf("Timeout.");
  signal(SIGALRM, timeout);
  longjmp(env, 1);
}

int main(int argc, char **agrv){
	struct sockaddr * ipaddr;
	ipaddr->sa_family=1;
	sprintf(ipaddr->sa_data,"%s","127.0.0.1");
	err_msg("%s",ipaddr->sa_data);
	socklen_t len=sizeof(struct sockaddr);
	struct hwaddr  *haddr;
	haddr->sll_ifindex=1;
	haddr->sll_hatype=2;
	haddr->sll_halen='1';
	areq(ipaddr,len,haddr);
}


#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>
#include "api.h"
#define SIZE 1024
#define CLI_SUN_PATH "g15_api_path"
#define ARP_DG_PATH "g15_arppath"

void printHW(char* ptr)
{
	int i = 6;
	do 
	{
		printf("%.2x%s", *ptr++ & 0xff, (i == 1) ? " " : ":");
	} while (--i > 0);
}

int areq (struct sockaddr *IPaddr, socklen_t sockaddrlen, struct hwaddr *HWaddr){
	
	int	sockfd;
	int	tempfd;
	int yes=1;
	char sendline[100];
	fd_set			rset;
	struct timeval	tv;
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	char tmp[100];
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
	
	err_msg(IPaddr->sa_data);
	strcpy(sendline,IPaddr->sa_data);
	sprintf(tmp,",%d,%u,%c",HWaddr->sll_ifindex,HWaddr->sll_hatype,HWaddr->sll_halen);
	strcat(sendline,tmp);
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
	err_msg("%s",sendline);
	Writen(sockfd, sendline, strlen(sendline));
	FD_ZERO(&rset);
	FD_SET(sockfd, &rset);
	int maxfd=sockfd;
	select(maxfd+1, NULL, &rset, NULL, &tv);
		if(FD_ISSET(sockfd,&rset)){
			recvfrom(sockfd, (struct hwaddr *)HWaddr, sizeof(struct hwaddr), 0, NULL, NULL);
			err_msg("read");
			err_msg("IP is");
			printHW(HWaddr->sll_addr);
			err_msg("Index is %d",HWaddr->sll_ifindex);

			}
	else{
		err_msg("API Timeout");
		return -1;
	}
	unlink(CLI_SUN_PATH);
	return 1;
}


//int main(int argc, char **agrv){
//	struct sockaddr * ipaddr;
//	ipaddr=(struct sockaddr *) malloc(sizeof(struct sockaddr));
//	ipaddr->sa_family=1;
//	strcpy(ipaddr->sa_data,"192.168.1.102");
//	err_msg("%s",ipaddr->sa_data);
//	socklen_t len=sizeof(struct sockaddr);
//	struct hwaddr  *haddr=malloc(sizeof(struct hwaddr));
//	haddr->sll_ifindex=2;
//	haddr->sll_hatype=1;
//	haddr->sll_halen='6';
//	int ret_val=areq(ipaddr,len,haddr);
//	err_msg("retval %d" , ret_val);	
//}

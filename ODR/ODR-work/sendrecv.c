#include <string.h>
#include <error.h>
#include <sys/socket.h>
#include <stdio.h>
#include "constants.h"
SA sockaddress;
socklen_t address_len;
int msg_send(int sockfd, char* address, int destport, char* message, int flag)
{
	err_msg("Inside MSG SEND");
	char sendline[100], port[7], flagstr[3];
	sendline[0]='\0';
	struct sockaddr_un su;
	strncat(sendline, address, 16);
	strcat(sendline, "-");
	sprintf(port, "%d", destport);
	strcat(sendline, port);
	//err_msg("15");
	strcat(sendline, "-");
	strncat(sendline, message, strlen(message) );
	strcat(sendline, "-");
	sprintf(flagstr, "%d", flag);
	//err_msg("20");
	strcat(sendline, flagstr);
	strcat(sendline, "\0");
	su.sun_family = AF_LOCAL;
	strcpy(su.sun_path, ODR_SUNPATH);
	//err_msg("25");
	err_msg("%s",su.sun_path);
	
	printf("Sending: %s\n", sendline);
	if(sendto(sockfd, (void *)sendline, sizeof(sendline), 0, (struct sockaddr*) &su, sizeof(struct sockaddr)) < 0)
	{
		printf("sendto error: %d %s\n", errno, strerror(errno));
		return -1;
	}
	
	return 0;
}

int msg_recv(int sockfd, char* message, char* address, int* port)
{
	char recvline[MAXLINE], *temp;
	char *result;
	 address_len=sizeof(sockaddress);
	//err_msg("43");
	if(recvfrom(sockfd, recvline, MAXLINE, 0, &sockaddress, &address_len) < 0)
	{
		printf("recvfrom error: %d %s\n", errno, strerror(errno));
		return -1;
	}

	result = strtok( recvline, "-" );

    strcpy(address,result);
    err_msg( "%s", address );
	
	result = strtok( NULL, "-");
	
	int portnum=atoi(result);
	*port=portnum;
    err_msg( "%d", *port );
	
	result = strtok( NULL, "-");
	message=result;
    err_msg( "%s", message );
	
	//result = strtok( NULL, "-");
	//err_msg( "%s", result );
	err_msg("###End of msg rev");
	
	return 0;
}
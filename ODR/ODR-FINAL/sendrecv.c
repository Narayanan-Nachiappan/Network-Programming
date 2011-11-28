#include <string.h>
#include <error.h>
#include <sys/socket.h>
#include <stdio.h>
#include "constants.h"
SA sockaddress;
socklen_t address_len;
int msg_send(int sockfd, char* address, int destport, char* message, int flag)
{
	char sendline[100], port[7], flagstr[3];
	sendline[0]='\0';
	struct sockaddr_un su;
	strncat(sendline, address, 16);
	strcat(sendline, "-");
	sprintf(port, "%d", destport);
	strcat(sendline, port);
	strcat(sendline, "-");
	strncat(sendline, message, strlen(message) );
	strcat(sendline, "-");
	sprintf(flagstr, "%d", flag);
	strcat(sendline, flagstr);
	strcat(sendline, "\0");
	su.sun_family = AF_LOCAL;
	strcpy(su.sun_path, ODR_SUNPATH);

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
	if(recvfrom(sockfd, recvline, MAXLINE, 0, &sockaddress, &address_len) < 0)
	{
		printf("recvfrom error: %d %s\n", errno, strerror(errno));
		return -1;
	}

	printf("Message received: %s\n", recvline);
	result = strtok( recvline, "-" );

    strcpy(address,result);
    err_msg( "Address: %s", address );
	
	result = strtok( NULL, "-");
	
	int portnum=atoi(result);
	err_msg( "Port: %d", portnum );
	
	*port=portnum;
	
	result = strtok( NULL, "-");
	message=result;
    err_msg( "Message: %s", message );
	
	return 0;
}
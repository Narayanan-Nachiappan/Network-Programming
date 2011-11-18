#include <string.h>
#include <error.h>
#include <sys/socket.h>
#include <stdio.h>

int msg_send(int sockfd, char* address, int destport, char* message, int flag)
{
	char sendline[100], port[7];
	struct sockaddr_un su;
	
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = destport;
	inet_pton(AF_INET, address, &dest_addr.sin_addr);
	
	strncat(sendline, address, sizeof(address));
	strcat(sendline, " ");
	sprintf(port, "%d", destport);
	strcat(sendline, port);
	strcat(sendline, " ");
	strncat(sendline, message, sizeof(message));
	strcat(sendline, "\0");
	
	su.sun_family = AF_LOCAL;
	strcpy(su.sun_path, TIME_SERV_SUNPATH);
	strcat(su.sun_path, "\0");
	
	if(sendto(sockfd, (void *)sendline, sizeof(sendline), 0, (struct sockaddr*) &su, sizeof(struct sockaddr)) < 0)
	{
		printf("sendto error: %d %s\n", errno, strerror(errno));
		return -1;
	}
	
	return 0;
}

int msg_recv(int sockfd, char* message, char* address, int* port)
{
	char *recvline, *temp;
	struct sockaddr address;
	socklen_t* address_len;

	if(recvfrom(sockfd, recvline, MAXLINE, 0, address, address_len) < 0)
	{
		printf("recvfrom error: %d %s\n", errno, strerror(errno));
		return -1;
	}
	
	message = strtok(NULL, " ");
	temp = strtok(NULL, " ");
	*port = atoi(temp);
	address = strtok(recvline, " ");
	

	return 0;
}
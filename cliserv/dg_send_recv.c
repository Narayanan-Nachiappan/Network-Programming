/* include dgsendrecv1 */
#include	"unprtt.h"
#include	<setjmp.h>
#include "struct.c"

#define	RTT_DEBUG


static struct rtt_info   rttinfo;
static int	rttinit = 0;
 int i=1;


static void	sig_alrm(int signo);
static sigjmp_buf	jmpbuf;
char prin[MAXLINE];



ssize_t
dg_send_recv_int(int fd, const char *outbuff, size_t outbytes,
			 char *inbuff, size_t inbytes,
			  SA *destaddr, socklen_t destlen)
{

}
/*
//proto = protocol type
//sendtype = type of send to use (sendto = 0, send = 1)
ssize_t
dg_send_recv(int fd, int fd2, const char *outbuff, size_t outbytes,
			 char *inbuff, size_t inbytes,
			  SA *destaddr, socklen_t destlen, int proto, int sendtype)
{
	ssize_t			n;
	ssize_t nbytes;
	int len, sock;
	struct iovec	iovsend[2], iovrecv[2];
	
	if (rttinit == 0) {
		rtt_init(&rttinfo);		
		rttinit = 1;
		rtt_d_flag = 1;
	}
	len=sizeof(destaddr);
	send_msg.seq++;
	send_msg.type = proto;
	strcpy(send_msg.data, outbuff);
	nbytes = sizeof(struct message);

	

	Signal(SIGALRM, sig_alrm);
	rtt_newpack(&rttinfo);		

sendagain:
#ifdef	RTT_DEBUG
	fprintf(stderr, "send %4d: ", send_msg.seq);
	
#endif
	send_msg.ts = rtt_ts(&rttinfo);
	
	if(sendtype == 0)
	{
		if(sendto(fd, (char *) &send_msg, nbytes, 0, destaddr, destlen) < 0)
			printf("\nsendto error %d %s\n", errno, strerror(errno));
	}
	else
		if(send(fd, (char *) &send_msg, nbytes, 0) < 0)
			printf("\nsend error %d %s\n", errno, strerror(errno));

	alarm(rtt_start(&rttinfo));	
#ifdef	RTT_DEBUG
	rtt_debug(&rttinfo);
#endif

	if (sigsetjmp(jmpbuf, 1) != 0) {
		if (rtt_timeout(&rttinfo) < 0) {
			err_msg("dg_send_recv: no response from client, giving up");
			rttinit = 0;	
			errno = ETIMEDOUT;
			return(-1);
		}
#ifdef	RTT_DEBUG
		err_msg("dg_send_recv: timeout, retransmitting");
		fprintf(stderr,"\nReducing window size to  %d",i++);
#endif
		goto sendagain;
	}

	do {
		if(sendtype == 0)
			sock = fd2;
		else
			sock = fd;
			
		if(recvfrom(sock, (struct message *)&recv_msg, MAXLINE, 0,  NULL, NULL) < 0)
			printf("\nrecvfrom error %d %s\n", errno, strerror(errno));
		//n = recv(fd, (struct message *)&recv_msg, MAXLINE, 0);
#ifdef	RTT_DEBUG

		

		
#endif
	} while ( recv_msg.seq != send_msg.seq);
	

	alarm(0);			
	rtt_stop(&rttinfo, rtt_ts(&rttinfo) - recv_msg.ts);
		fprintf(stderr,"\nReceived ack for datagram %d ",recv_msg.seq);
		fprintf(stderr,"\nReceived data %s ",recv_msg.data);
	if(i<recv_msg.wind_size){
		fprintf(stderr,"\nIncreased window size %d ",i++);}
	return(recv_msg.seq);	
}
*/
ssize_t
dg_send_recv(int fd, int fd2, const char *outbuff, size_t outbytes,
char *inbuff, size_t inbytes,
 SA *destaddr, socklen_t destlen, int proto, int sendtype)
{
ssize_t n;
ssize_t nbytes;
int len, sock;
struct iovec iovsend[2], iovrecv[2];
if (rttinit == 0) {
rtt_init(&rttinfo); /* first time we're called */
rttinit = 1;
rtt_d_flag = 1;
}
len=sizeof(destaddr);
send_msg.seq++;
strcpy(send_msg.data, outbuff);
nbytes = sizeof(struct message);

/* include dgsendrecv2 */
Signal(SIGALRM, sig_alrm);
rtt_newpack(&rttinfo); /* initialize for this packet */

sendagain:
#ifdef RTT_DEBUG
fprintf(stderr, "send %4d: ", send_msg.seq);
#endif
send_msg.ts = rtt_ts(&rttinfo);
send_msg.type=proto;
if(sendtype == 0)
{
if(sendto(fd, (char *) &send_msg, nbytes, 0, destaddr, destlen) < 0)
printf("\nsendto error %d %s\n", errno, strerror(errno));
}
else
if(send(fd, (char *) &send_msg, nbytes, 0) < 0)
printf("\nsend error %d %s\n", errno, strerror(errno));

alarm(rtt_start(&rttinfo)); /* calc timeout value & start timer */
#ifdef RTT_DEBUG
rtt_debug(&rttinfo);
#endif

if (sigsetjmp(jmpbuf, 1) != 0) {
if (rtt_timeout(&rttinfo) < 0) {
err_msg("dg_send_recv: no response from client, giving up");
rttinit = 0; /* reinit in case we're called again */
errno = ETIMEDOUT;
return(-1);
}
#ifdef RTT_DEBUG
err_msg("dg_send_recv: timeout, retransmitting");
if(i>0){
fprintf(stderr,"\nReducing window size to  %d",i--);
}
#endif
goto sendagain;
}

do {
if(sendtype == 0)
sock = fd2;
else
sock = fd;
if(recvfrom(sock, (struct message *)&recv_msg, MAXLINE, 0,  NULL, NULL) < 0)
printf("\nrecvfrom error %d %s\n", errno, strerror(errno));
//n = recv(fd, (struct message *)&recv_msg, MAXLINE, 0);
#ifdef RTT_DEBUG


#endif
} while ( recv_msg.seq != send_msg.seq);

alarm(0); /* stop SIGALRM timer */
/* 4calculate & store new RTT estimator values */
rtt_stop(&rttinfo, rtt_ts(&rttinfo) - recv_msg.ts);
fprintf(stderr,"\nReceived ack for datagram %d ",recv_msg.seq);
fprintf(stderr,"\nReceived data %s ",recv_msg.data);
if(i<recv_msg.wind_size){
fprintf(stderr,"\nIncreased window size %d ",i++);}
return(recv_msg.seq); /* return size of received datagram */
}

static void
sig_alrm(int signo)
{
	siglongjmp(jmpbuf, 1);
}
/* end dgsendrecv2 */

ssize_t
Dg_send_recv(int fd, int fd2, const char *outbuff, size_t outbytes,
			 char *inbuff, size_t inbytes,
			  SA *destaddr, socklen_t destlen, int proto, int sendtype)
{
	ssize_t	n;


	n = dg_send_recv(fd, fd2, outbuff, outbytes, inbuff, inbytes,
					 destaddr, destlen, proto, sendtype);

	if (n < 0)
		err_quit("dg_send_recv error");

	return(n);
}

/*
void dg_echofun(FILE * fp,int sockfd, SA *pcliaddr, socklen_t clilen)
{
	int i=1;
	socklen_t	len;
	char		mesg[MAXLINE];
	ssize_t	n;
	char	sendline[MAXLINE], recvline[MAXLINE + 1],prin[MAXLINE + 1];
	fprintf(stderr, "Slow start#### Init wind size %4d ", i);
	while (Fgets(sendline, MAXLINE, fp) != NULL) {
		//Fputs(sendline,stdout);
		//fprintf(stderr, "%s", sendline);
		n = Dg_send_recv(sockfd, 0, sendline, strlen(sendline),
						 recvline, MAXLINE, pcliaddr, clilen, 3, 1);
		
		recvline[n] = 0;
		
		
		Fputs(prin,stdout);
		fflush(stdout);
		//Fputs(recvline,stdout);
		//fflush(stdout);
	}

}*/

void dg_echofun(FILE * fp,int sockfd, const SA *pcliaddr, socklen_t clilen)
{
socklen_t len;
char mesg[MAXLINE];
ssize_t n;
int send=1;
    int i=0,eof=3;
char c;
int k=0;
char sendline[496], recvline[MAXLINE + 1];
strcpy(sendline,"");
  while(1){
 
  while (i<496){
    c=getc(fp);
if(c==EOF){
i++;
eof=6;
goto end;
}
else{
sendline[i]=c;
i++;
}
  }
sendline[i]='\0';
  //fprintf(stderr,"%s",sendline);
  n = Dg_send_recv(sockfd,0, sendline, strlen(sendline),
recvline, MAXLINE, pcliaddr, clilen,eof,send);
  i=0;
  for(k=0;k<496;k++){
 sendline[k]='\0';
  }
  }
end:
sendline[i]='\0';
//fprintf(stderr,"%s",sendline);
n = Dg_send_recv(sockfd,0, sendline, strlen(sendline),
recvline, MAXLINE, pcliaddr, clilen,eof,send);
err_msg("End OF FIle");
err_msg(" ");
} 
/* include dgsendrecv1 */
#include	"unprtt.h"
#include	<setjmp.h>
#include "struct.c"
#include "rtt.c"

#define	RTT_DEBUG


static struct rtt_info   rttinfo;
static int	rttinit = 0;
 int i=1;
 int loop=0;
static void	sig_alrm(int signo);
static sigjmp_buf	jmpbuf;
char prin[MAXLINE];
int count=1,all=1;

void printMessage(struct message msg){
	err_msg("----------------------------------------");
	err_msg("Packet received");
	err_msg("Time Stamp: %d", msg.ts);
	err_msg("Sequence#: %d", msg.seq);
	err_msg("Type: %d", msg.type);
	err_msg("Advertised Window size: %d", msg.wind_size);
	err_msg("Message: %s", msg.data);
}



ssize_t
dg_send_recv_int(int fd, const char *outbuff, size_t outbytes,
			 char *inbuff, size_t inbytes,
			  SA *destaddr, socklen_t destlen)
{

}

//---------------------------selective ARQ
ssize_t
dg_send_recv_selective(int fd, int fd2, const char *outbuff, size_t outbytes,
char *inbuff, size_t inbytes,
 SA *destaddr, socklen_t destlen, int proto, int windsize)
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
if(count==windsize){
Signal(SIGALRM, sig_alrm);
rtt_newpack(&rttinfo); /* initialize for this packet */
}
sendagain:
#ifdef RTT_DEBUG
fprintf(stderr, "send datagram %4d: \n ", send_msg.seq);
err_msg("Checking sender window\n");
#endif
send_msg.ts = rtt_ts(&rttinfo);
send_msg.type=proto;

if(send(fd, (char *) &send_msg, nbytes, 0) < 0)
printf("\nsend error %d %s\n", errno, strerror(errno));

if(count==windsize){
alarm(rtt_start(&rttinfo)); /* calc timeout value & start timer */
#ifdef RTT_DEBUG
rtt_debug(&rttinfo);
#endif
}
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

sock = fd;
if(recvfrom(sock, (struct message *)&recv_msg, MAXLINE, 0,  NULL, NULL) < 0)
printf("\nrecvfrom error %d %s\n", errno, strerror(errno));
//n = recv(fd, (struct message *)&recv_msg, MAXLINE, 0);
#ifdef RTT_DEBUG


#endif

} while ( recv_msg.seq != send_msg.seq);

if(count==windsize){
alarm(0); /* stop SIGALRM timer */
/* 4calculate & store new RTT estimator values */
rtt_stop(&rttinfo, (int)(rtt_ts(&rttinfo) - recv_msg.ts));
//fprintf(stderr,"\nReceived ack for datagram %d ",recv_msg.seq);
//printMessage(recv_msg);
//fprintf(stderr,"\nReceived data %s ",recv_msg.data);
}
if(count==windsize){
	
	err_msg("Sender window full, Reading ACKs");
	for(loop=(all-count);loop<=all;loop++){
		err_msg("Read ACK for packet %d from buffer",loop+1);
	}
	count=1;
	}
	if(proto==6){
		err_msg("End OF FIle");
		err_msg(" ");
	count=1;
	
	for(loop=(all-count);loop<=all;loop++){
		err_msg("Read ACK for packet %d from buffer",loop+1);
	}
	}
else{
	count++;
	all++;
}
return(recv_msg.seq); /* return size of received datagram */
}

//----stop and wait


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
rtt_stop(&rttinfo, (int)(rtt_ts(&rttinfo) - recv_msg.ts));
fprintf(stderr,"\nReceived ack for datagram %d ",recv_msg.seq);
printMessage(recv_msg);
//fprintf(stderr,"\nReceived data %s ",recv_msg.data);

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
			  SA *destaddr, socklen_t destlen, int proto, int sendtype, int windowtype, int maxwindowsize)
{
	ssize_t	n;

	int windtype= windowtype;
	int windsize= maxwindowsize;
	if(windtype==1)
	n = dg_send_recv(fd, fd2, outbuff, outbytes, inbuff, inbytes,
					 destaddr, destlen, proto, sendtype);

else if(windtype==2)
n = dg_send_recv_selective(fd, fd2, outbuff, outbytes, inbuff, inbytes,
					 destaddr, destlen, proto, windsize);

	if (n < 0)
		err_quit("dg_send_recv error");

	return(n);
}



void dg_echofun(FILE * fp,int sockfd, const SA *pcliaddr, socklen_t clilen, int windowtype, int maxwindowsize)
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
  //fprintf(stderr,"\nSlow start to avoid congestion on network with size 1\n");
  n = Dg_send_recv(sockfd,0, sendline, strlen(sendline),
recvline, MAXLINE, pcliaddr, clilen,eof,send, windowtype,  maxwindowsize);
  i=0;
  for(k=0;k<496;k++){
 sendline[k]='\0';
  }
  }
end:
sendline[i]='\0';
//fprintf(stderr,"%s",sendline);
n = Dg_send_recv(sockfd,0, sendline, strlen(sendline),
recvline, MAXLINE, pcliaddr, clilen,eof,send, windowtype,  maxwindowsize);
err_msg("End OF FIle");
err_msg(" ");
} 
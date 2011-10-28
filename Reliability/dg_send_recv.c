/* include dgsendrecv1 */
#include	"unprtt.h"
#include "struct.c"
#include	<setjmp.h>

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

ssize_t
dg_send_recv(int fd, const char *outbuff, size_t outbytes,
			 char *inbuff, size_t inbytes,
			  SA *destaddr, socklen_t destlen)
{
	ssize_t			n;
	ssize_t nbytes;
	int len;
	struct iovec	iovsend[2], iovrecv[2];

	if (rttinit == 0) {
		rtt_init(&rttinfo);		/* first time we're called */
		rttinit = 1;
		rtt_d_flag = 1;
	}
	len=sizeof(destaddr);
	send_msg.seq++;
	strcpy(send_msg.data, outbuff);
	nbytes = sizeof(struct message);
	
/* include dgsendrecv2 */
	Signal(SIGALRM, sig_alrm);
	rtt_newpack(&rttinfo);		/* initialize for this packet */

sendagain:
#ifdef	RTT_DEBUG
	fprintf(stderr, "send %4d: ", send_msg.seq);
	
#endif
	send_msg.ts = rtt_ts(&rttinfo);
	Sendto(fd, (char *) &send_msg, nbytes, 0, destaddr, destlen);

	alarm(rtt_start(&rttinfo));	/* calc timeout value & start timer */
#ifdef	RTT_DEBUG
	rtt_debug(&rttinfo);
#endif

	if (sigsetjmp(jmpbuf, 1) != 0) {
		if (rtt_timeout(&rttinfo) < 0) {
			err_msg("dg_send_recv: no response from client, giving up");
			rttinit = 0;	/* reinit in case we're called again */
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
		n = Recvfrom(fd, (struct message *)&recv_msg, MAXLINE, 0,  NULL, NULL);
#ifdef	RTT_DEBUG

		

		
#endif
	} while ( recv_msg.seq != send_msg.seq);
	

	alarm(0);			/* stop SIGALRM timer */
		/* 4calculate & store new RTT estimator values */
	rtt_stop(&rttinfo, rtt_ts(&rttinfo) - recv_msg.ts);
		fprintf(stderr,"\nReceived ack for datagram %d ",send_msg.seq);
	if(i<recv_msg.wind_size){
		fprintf(stderr,"\nIncreased window size %d ",i++);}
	return(recv_msg.seq);	/* return size of received datagram */
}

static void
sig_alrm(int signo)
{
	siglongjmp(jmpbuf, 1);
}
/* end dgsendrecv2 */

ssize_t
Dg_send_recv(int fd, const char *outbuff, size_t outbytes,
			 char *inbuff, size_t inbytes,
			  SA *destaddr, socklen_t destlen)
{
	ssize_t	n;


	n = dg_send_recv(fd, outbuff, outbytes, inbuff, inbytes,
					 destaddr, destlen);

	if (n < 0)
		err_quit("dg_send_recv error");

	return(n);
}

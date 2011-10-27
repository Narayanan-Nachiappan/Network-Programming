
static struct hdr {
  uint32_t	seq;	/* sequence # */
  uint32_t	ts;
  char	data[MAXLINE];		/* timestamp when sent */
  
} sendhdr, recvhdr;
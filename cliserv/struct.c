static struct message {
  uint32_t	seq;	/* sequence # */
  uint32_t	ts;
  unit32_t	type;
  char	*data;		/* timestamp when sent */
  
} sendmsg, recvmsg;
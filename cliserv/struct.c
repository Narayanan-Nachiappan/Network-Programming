
static struct message {
  uint32_t	seq;	/* sequence # */
  uint32_t	ts;
  uint32_t wind_size;
  uint32_t	type;
  char	data[MAXLINE];		/* timestamp when sent */
  
} send_msg, recv_msg;
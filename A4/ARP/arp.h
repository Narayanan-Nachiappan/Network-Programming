#include <errno.h>
#include "hw_addrs.h"
#include <linux/if_ether.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netpacket/packet.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <time.h>

#define MAX_INTERFACES 10
#define MAX_ARP_ITEMS 10
#define ARP_DG_PATH "g15_arppath"
#define PROTO_TYPE 12345
#define ID_NUM 0x4354
struct interface
{
	char    if_haddr[6];	/* hardware address */
	int     if_index;		/* interface index */
	char	ip_addr[16];	/* IP address */
};

struct arp_cache
{
		char	ip_addr[16];	/* IP address */
		char    if_haddr[6];	/* hardware address */
		int     if_index;		/* interface index */
		unsigned short  sll_hatype; /*Hardware Type*/
		int		sockfd;

};

struct hwaddr 
{
		     int             sll_ifindex;	 /* Interface number */
		     unsigned short  sll_hatype;	 /* Hardware type */
		     unsigned char   sll_halen;		 /* Length of address */
		     unsigned char   sll_addr[8];	 /* Physical layer address */
};

struct arp_message
{
	short int id;
	int hard_type;
	int proto_type;
	int op;
	int hard_size;
	int proto_size;
	char sender_ip[16];
	char sender_haddr[6];	
	char target_ip[16];
	char target_haddr[6];
};

struct arp_cache arp_table[MAX_ARP_ITEMS];
struct interface interfaces[MAX_INTERFACES];
int tablesize;

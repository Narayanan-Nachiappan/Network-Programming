#define MAX_INTERFACES 10
#define MAX_ARP_ITEMS 10
#define ARP_DG_PATH "g15_arppath"
#define PROTO_TYPE 12345
struct interface
{
	char    if_haddr[6];	/* hardware address */
	int     if_index;		/* interface index */
	char	ip_addr[16];	/* IP address */
};

struct arp_message
{
		char    if_haddr[6];	/* hardware address */
		int     if_index;		/* interface index */
		char	ip_addr[16];	/* IP address */
		unsigned short  sll_hatype; /*Hardware Type*/

};

struct  arp_message arp_table[MAX_ARP_ITEMS];

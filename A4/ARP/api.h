#include "unp.h"

struct hwaddr {
		     int             sll_ifindex;	 /* Interface number */
		     unsigned short  sll_hatype;	 /* Hardware type */
		     unsigned char   sll_halen;		 /* Length of address */
		     unsigned char   sll_addr[8];	 /* Physical layer address */
};

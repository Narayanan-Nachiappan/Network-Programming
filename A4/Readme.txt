CSE 533 - Network Programming
Assignment 4

Group# 15
	Amelia Ellison - 107838108
	Narayanan Nachiappan - 107996031
	Youngbum Kim - 107387376

----------------------------------------
1. Command to use to obtain

tar xvzf cse533_group15_assignment4.tar.gz

To compile code:

make

----------------------------------------
2. Source files

/** Tour
* passes of rt message
* Sends echo requests and receives the reply
* queries ARP for h/w addr through API
*/
Tour.c

/*API*/
api.c

/*ARP*/
arp.c.c

/* api header 
*  Contains the structures arpcache and hwaddr
*/
api.h

/*Tour header that contains the Tour message structure*/
a.h

/* Makefile */
Makefile

/* This file */
Readme.txt

----------------------------------------
API

	--This receives the sockaddr structure from TOUR application that contains the IP for which the hardware
	addr is requested. 
	--The API inturn uses domain socket [STREAM] to pass the ip details to ARP and waits 
	for its reply. 
	--If the reply arrives , it is returned to the TOUR application. Or else the API receive times out

_______________________
MULTICAST send/recv
-	Multicast receive
	--When an intermediate node receives the rt [tour message] apart from updating and passing to the next node
	it also reads the multicast ip and port number and joins the multicast group using
	IP_ADD_MEMBERSHIP option on a UDP socket.
	--Then it waits on the multicast socket for messages to arrive on a select.
-	Multicast send
	When the last node in the tour is encountered , it sends a message on to the 
	multicast socket which is followed by a reply from all ther intermediate nodes on the 
	multicst socket
________________________
TOUR
       --Source node (which is executed with arguments)
         Tour parses input arguments and create list of nodes which will be visited.
         Tour sends IP Raw Packet through the rt socket to the next visiting node with payload (includes, tour list,
        Multicast group info)

       --The other nodes(which is executed with no argument)
         When the node received RT packet with the tour list. the nodes join the multicast group and ping the source
       node(ICMP request) ( now the tour communicates with arp through API )
         The last node of the tour list will send multicast message asking other nodes to identify itself.

       --When each nodes receives multicast message asking indentification, it sends multicast packet identifying
       itself.
 _________________________
 ARP

The ARP process keeps track of the hardware addresss of its interfaces and any nodes that contact it in the ARP table.
We've implemented the ARP table as an array of arp_cache structures, as defined below. The ARP process is called by the API.
After checking it's table for the entry, if it has the entry it responds. If not, it broadcasts a request to the other nodes.
Nodes that have the entry will reply and the requesting node will update it's table and send a hwaddr structure to the local
tour node via the API.

struct arp_cache
{
		char	ip_addr[16];	/* IP address */
		char    if_haddr[6];	/* hardware address */
		int     if_index;		/* interface index */
		unsigned short  sll_hatype; /*Hardware Type*/
		int		sockfd; /*UNIX socket description */
};
____________________________________________
PING

The ping echo is constructed by filling in ethernet, IP, and ICMP headers and sending the packet to the proper destination.
Using the IP_HDRINCL option tells the kernel not to add it's own IP header. The ECHO request is responded to automatically by
the kernel on the receiving machine. On the first machine, we can parse out the values and make sure that the ping originated
from us by checking the ID value.
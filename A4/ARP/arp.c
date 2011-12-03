#include	"unp.h"
#include "arp.h"
#include "get_hw_addrs.c"



void printHW(char* );
void print_ip_hw_pairs();
int
main(int argc, char **argv)
{
	int pfsock,domainsock;
	struct sockaddr_un	servaddr, cliaddr;
	int maxfd;
	fd_set fdset;
	/*
	*Prints the IP,HW addr pair for all aliases for interface eth0
	*/
	print_ip_hw_pairs();

	/*
	*Domain socket for communicating with local tour
	*/
	domainsock = Socket(AF_LOCAL, SOCK_STREAM, 0);
	unlink(ARP_DG_PATH);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sun_family = AF_LOCAL;
	strcpy(servaddr.sun_path, ARP_DG_PATH);
	Bind(domainsock, (SA *) &servaddr, sizeof(servaddr));
	
	/*
	*Raw PF_PACKET socket to get the HW addr on request
	*/
	pfsock=socket(PF_PACKET, SOCK_RAW,ntohs(PROTO_TYPE));
	
	/*
	*Init for socket descriptor set
	*/
	FD_ZERO(&fdset);

	/*
	*Infinite loop that selects between domain and raw socket
	*/
	for(;;){
			FD_SET(pfsock, &fdset);
			FD_SET(domainsock, &fdset);
			maxfd = max(pfsock, domainsock) + 1;
			printf("Selecting...\n");
			if(select(maxfd, &fdset, NULL, NULL, NULL) < 0)
			{
				printf("select error: %d %s\n", errno, strerror(errno));
			}
			
			/*
			* Data on packet socket RAW
			*/
			if(FD_ISSET(pfsock,&fdset)){

			}
			
			/*
			* Data on domain
			*/
			if(FD_ISSET(domainsock,&fdset)){

			}

	}

	}



void printHW(char* ptr)
{
	int i = 6;
	do 
	{
		printf("%.2x%s", *ptr++ & 0xff, (i == 1) ? " " : ":");
	} while (--i > 0);
}

void print_ip_hw_pairs(){
		struct hwa_info			*hwa, *hwahead;
		struct sockaddr			*sa;
		int i;
		struct interface		interfaces[MAX_INTERFACES];
		printf("interfaces IP Address,HW address pair \n");
		printf("_________________________________________________________\n");
		printf("IPAddress\t\tHWAddr\n");
		printf("_________________________________________________________\n");

		for (i = 0, hwahead = hwa = Get_hw_addrs(); hwa != NULL && i < MAX_INTERFACES; hwa = hwa->hwa_next, i++)
		{
			if(strncmp(hwa->if_name, "eth0", 4) != 0 && strncmp(hwa->if_name, "lo", 2) != 0)
			{
				/*
				for(j = 0; j < 6; j++)
				{
					interfaces[i].if_haddr[j] = hwa->if_haddr[j];
				}*/
				memcpy((void*)interfaces[i].if_haddr, (void*)hwa->if_haddr, 6);
				interfaces[i].if_index = hwa->if_index;
				sa = hwa->ip_addr;
				strncpy(interfaces[i].ip_addr, sock_ntop_host(sa, sizeof(*sa)), 16);
				printf("\n%s\t\t",  interfaces[i].ip_addr);
				printHW(interfaces[i].if_haddr);
				//printf("\nIndex: %d\n\n", interfaces[i].if_index);
			}
			else
				i--;
		}
	printf("\n___________________________________________________________\n");
}


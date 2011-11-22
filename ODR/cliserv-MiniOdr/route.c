#include "odr.h"

static int freeslot;


struct route newRoute(char* address, char* haddr, int index, int hops)
{
	struct route r;

	strncpy(r.ip, address, 16);
	strncpy(r.nexthop, haddr, 6);
	r.index = index;
	r.hops = hops;
	r.timestamp = time(NULL);

	return r;
}

int findInTable(char *address)
{
	int i;
	for(i = 0; i < MAX_ROUTES; i++)
	{
		if(strncmp(address, routing_table[i].ip, 16) == 0)
		{
			return i;
		}
	}
	return -1;
}

int findSlot()
{
	int i;

	if(freeslot < MAX_ROUTES)
	{
		freeslot++;
		return freeslot - 1;
	}
	else
		return -1;
}

int gotFreshRoute(char *address, int staleness)
{
	int i;
	struct sockaddr *sa;
	char interface_addr[16];
	time_t curr_time = time(NULL);
	
	i = findInTable(address);
	
	if(i > 0)
	{
		if(routing_table[i].timestamp == 0)
		{	
			printf("No route\n");
			return -1;
		}
		if(difftime(routing_table[i].timestamp, curr_time) < staleness)
		{
			printf("Got fresh route\n");
			return i;
		}
		else
		{
			deleteRoute(i);
			printf("Got stale route, deleting\n");
			return -1;
		}
	}
	
	printf("No route for %s\n", address);
	return -1;
}

int deleteRoute(int index)
{
	routing_table[index].timestamp = 0;
}

int updateTable(char* address, char* haddr, int index, int hops,int routediscovery)
{
	int i;
	
	i = findInTable(address);
	if((i == findInTable(address)) > 0) //If it's in the table, update it
	{
		if(routediscovery==1){
			strncpy(routing_table[i].nexthop, haddr, 6);
			routing_table[i].index = index;
			routing_table[i].hops = hops;
			routing_table[i].timestamp = time(NULL);
		}
		else if(routing_table[i].hops > hops)
		{
			strncpy(routing_table[i].nexthop, haddr, 6);
			routing_table[i].index = index;
			routing_table[i].hops = hops;
			routing_table[i].timestamp = time(NULL);
		}
		
	}

}

void rtable_init()
{
	int i;
	freeslot = 0;
	for(i = 0; i < MAX_ROUTES; i++)
	{
		routing_table[i].ip[0] = '\0';
		routing_table[i].nexthop[0] = '\0';
		routing_table[i].index = 0;
		routing_table[i].hops = 0;
		routing_table[i].timestamp = 0;
	}
}
int findFreeSpot(){
	int i;
	for(i = 0; i < MAX_ROUTES; i++)
	{
		if(routing_table[i].ip[0]=='\0'){
			return i;
		}
	}
	 
		return -1;
}

void rtable_display()
{
	int i;
	freeslot = 0;
	err_msg("__________________________________________________________");
	err_msg("************************ROUTING TABLE**************************************");
	err_msg("__________________________________________________________");
	err_msg("IP\t\tNEXTHOP\tINDEX\tHOPCOUNT\tTIMESTAMP");
	for(i = 0; i < MAX_ROUTES; i++)
	{
		err_msg("%s\t%s\t%d\t%d\t\t%d",routing_table[i].ip,routing_table[i].nexthop,routing_table[i].index,routing_table[i].hops,routing_table[i].timestamp );
		err_msg("\n");
	}
	err_msg("__________________________________________________________");
	err_msg("__________________________________________________________");
}

void rtable_add(struct route entry,int index)
{
	routing_table[index]=entry;
}
/*
int main(int argc,char **argv){
	char ip[16];
	char hop[6];
	int find_in_table,free_spot_in_table,hopcount,routediscovery;
	struct route route_entry;
	rtable_init();
	rtable_display();
	while(1){
		err_msg("Enter IP to add");
		scanf("%s",ip);
		err_msg("Enter Hop ");
		scanf("%s",hop);
		err_msg("Enter Hop count");
		scanf("%d",&hopcount);
		err_msg("Enter Force update val");
		scanf("%d",&routediscovery);
		
		find_in_table=findInTable(ip);
		if(find_in_table ==-1){
			free_spot_in_table=findFreeSpot();
			if(free_spot_in_table==-1){
				err_msg("Table full");
			}
			else{
\
				route_entry=newRoute(ip,hop,free_spot_in_table,hopcount);
				rtable_add(route_entry,free_spot_in_table);
					}
			}
		else{
			updateTable(ip,hop,find_in_table,hopcount,routediscovery);

		}
		rtable_display();
	  }
}*/

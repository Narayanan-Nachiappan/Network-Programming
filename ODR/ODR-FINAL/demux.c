#include "odr.h"

void dtable_init()
{
	int i = 0;
	headdemux = malloc(sizeof(struct demux));
	dtablesize = 1;

	//put the server in the table
	headdemux->port = SERVER_UNIX_PORT;
	strncpy(headdemux->sun_path, SERVER_UNIX_DG_PATH, sizeof(SERVER_UNIX_DG_PATH));
	headdemux->timestamp = time(NULL);
	printf("Demux Table: Added <%d, %s>\n", headdemux->port, headdemux->sun_path);
	lastdemux = headdemux;
}

void addnew(struct demux *newdemux)
{
	lastdemux->next = newdemux;
	lastdemux = newdemux;
}

void addNewDemux(int port, char* sp)
{
	char portstring[7], sun_path[20];
	
	//add <port, sunpath> pair to table
	struct demux *newentry = malloc(sizeof(struct demux));
	portstring[0] = '\0';
	sun_path[0] = '\0';
	
	newentry->port = port;
	strncpy(newentry->sun_path, sp, 20);
	
	newentry->next = NULL;
	newentry->timestamp = time(NULL);
	addnew(newentry);
	dtablesize++;
}


void removedemux(struct demux *rmdemux)
{
	struct demux *current, *previous;
	
	//You should never erase the head of the table (permanent server entry)
	for(current = headdemux->next, previous = headdemux; current != NULL; previous = current, current = current->next)
	{
		if(current == rmdemux)
		{
			if(lastdemux == rmdemux)
			{
				lastdemux = previous;
				lastdemux->next = NULL;
			}
			else
				previous->next = current->next;
			
			free(current);
			dtablesize--;
			printf("Removed demux, %d demux entries in table\n", dtablesize);
		}
	}
}

void updatetime(struct demux* d)
{
	d->timestamp = time(NULL);
}

struct demux* inDemuxTable(int port)
{
	struct demux *current;
	
	//if the value is in the table, return the pointer to the table entry
	printf("Looking for %d\n", port);
	for(current = headdemux; current != NULL; current = current->next)
	{
		if(current->port == port)
		{
			printf("It's in the demux table\n");
			return current;
		}
	}
	
	printf("It's not in the demux table\n");
	return NULL;
}

void purge(int staleness)
{
	struct demux *current, *previous, *temp;
	time_t curr_time = time(NULL);
	
	//remove old entries
	for(current = headdemux->next; current != NULL;)
	{
		if(difftime(current->timestamp, curr_time) > staleness)
		{
			temp = current;
			current = current->next;
			removedemux(temp);
		}
		else
			current = current->next;
	}	
}

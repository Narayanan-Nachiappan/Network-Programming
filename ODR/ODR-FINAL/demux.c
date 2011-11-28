#include "odr.h"

void dtable_init()
{
	int i = 0;
	headdemux = malloc(sizeof(struct demux));
	dtablesize = 1;
	//put the server in the table
	//strncpy(sunpath_root, SUNPATH_ROOT, sizeof(SUNPATH_ROOT));
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
	
	struct demux *newentry = malloc(sizeof(struct demux));
	portstring[0] = '\0';
	sun_path[0] = '\0';
	//sprintf(portstring, "%d", port);
	//strncpy(sun_path, SUNPATH_ROOT, 7); //cut off the \0
	//strncat(sun_path, portstring, strlen(portstring));
	//sprintf(sun_path, "%s%d\0", SUNPATH_ROOT, port);
	//printf("Sunpath_root: %s,  Portstring %s\n", sunpath_root, portstring);
	
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
	
	printf("Look for %d\n", port);
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


void purge(int staleness) //figure out when to call this!
{
	struct demux *current, *previous, *temp;
	time_t curr_time = time(NULL);
	
	for(current = headdemux->next; current != NULL;)
	{
		if(difftime(current->timestamp, curr_time) > staleness)
		{
			temp = current;
			current = current->next;
			removedemux(temp); //This could be wrong, remove calls free on this pointer...?
		}
		else
			current = current->next;
	}	
}

/*
int main(int argc, char **argv)
{
	char choice[7];
	int port, i;
	struct demux* temp;
	
	dtable_init();
	
	for( ; ;)
	{	
		printf("insert/remove/print: ");
		scanf("%s", choice);
	
		if(strncmp(choice, "insert", 6) == 0)
		{
			printf("Port Number: ");
			scanf("%d", &port);
			temp = inDemuxTable(port);
			if(temp == NULL)
				addNewDemux(port);
			else
					printf("Already in table\n");
		}
		else if(strncmp(choice, "remove", 6) == 0)
		{
			printf("Port Number: ");
			scanf("%d", &port);
			temp = inDemuxTable(port);
			if(temp != NULL)
				removedemux(temp);
			
		}
		else if(strncmp(choice, "print", 5) == 0)
		{
			for(temp = headdemux; temp != NULL; temp = temp->next)
			{
				printf("<%d, %s>\n", temp->port, temp->sun_path);
			}
		}
	}
}
*/

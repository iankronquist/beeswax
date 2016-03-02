#define _XOPEN_SOURCE 600 //Needed to compile without Errors
#include <assert.h>
#include <errno.h>
#include <ftw.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <time.h>

#include "znotify.h"

#define OPT_W 0 // Watch Inputed Directories Only
#define OPT_T 1 // Traverse Inputed Directories
#define OPT_A 2 // Add New Directories to Watch List
#define OPT_N 3 // New Directories/Files are Reported Only
#define OPT_E 4 // Every Event is Reported

// Defined in limits.h
const char *in_access_cstring           = "IN_ACCESS";
const char *in_modify_cstring           = "IN_MODIFY";
const char *in_attrib_cstring           = "IN_ATTRIB";
const char *in_close_write_cstring      = "IN_CLOSE_WRITE";
const char *in_close_no_write_cstring   = "IN_CLOSE_NOWRITE";
const char *in_open_cstring             = "IN_OPEN";
const char *in_moved_from_cstring       = "IN_MOVED_FROM";
const char *in_moved_to_cstring         = "IN_MOVED_TO";
const char *in_create_cstring           = "IN_CREATE";
const char *in_delete_cstring           = "IN_DELETE";
const char *in_delete_self_cstring      = "IN_DELETE_SELF";
const char *folder_cstring              = "FOLDER";
const char *file_cstring                = "FILE";

struct znotify steve;
int mask;

static void handle_events(int fd, int *wd);
static void json_safe(const char * source, char * destination, int size);
static void print_json(const char * date, const char * event,const char *directory,
                       const char *name, const char *type);
static void fetch_argument(int *i, int argc, char *argv[], char **voltar);
static nfds_t count_arguments(int argc, char *argv[]);
static int watch_this(const char *pathname);
static void help_menu(char* file);
static int walker(const char *pathname, const struct stat *sbuf, 
	          int type,struct FTW *ftwb);


int main(int argc, char* argv[])
{
	int i; 
	int j;
	int poll_num;
	char *file_name;

	short options[5] = {0,0,0,1,0};
	struct pollfd *poll_fd; 
	if(argc == 1)
	{
		fprintf(stderr,"\nNot Enough Arguments");
		help_menu(argv[0]);
		exit(EXIT_FAILURE);
	}
	while( (i = getopt(argc, argv, "w:t:hane")) != -1)
	{
		switch(i)
		{
			case 'w':
				options[OPT_W] = 1;
				break;
			case 't':
				options[OPT_T] = 1;
				break;
			case 'h':
				help_menu(argv[0]);
				exit(EXIT_SUCCESS);			
				break;
			case 'a':
				options[OPT_A] = 1;
				break;
			case 'n':
				options[OPT_N] = 1;
				break;
			case 'e':
				options[OPT_E] = 1;
				break;
			case ':':
				fprintf(stderr,"Missing Directory\n");
				exit(EXIT_FAILURE);
				break;
			case '?':
				fprintf(stderr,"Use: %s -h\n",argv[0]);
				exit(EXIT_FAILURE);
				break;
			default:
				fprintf(stderr,"\nInvalid Choice: %s -h",argv[0]);
				exit(EXIT_FAILURE);
		}
	}
	steve.arguments = count_arguments(argc,argv);
	steve.fd = calloc(steve.arguments, sizeof(int));
	steve.w_count = calloc(steve.arguments,sizeof(int));
	steve.wd = calloc(steve.arguments, sizeof(int*));
	
	poll_fd = calloc(steve.arguments,sizeof(struct pollfd));

	for(i = 0; i < steve.arguments; i++)
	{
		steve.w_count[i] = DEFAULT_WATCH_NUMBER;
		steve.wd[i] = calloc(DEFAULT_WATCH_NUMBER,sizeof(int));
	}

	if(OPT_N) //Report Only New Directories
	{
		mask = IN_CREATE;
	}
	else
	{
		mask = IN_ALL_EVENTS;
	}
	/* Can Only Watch Listed Directories or Traverse them, not both */
	if(options[OPT_W])
	{
		for(i=0,j=1; i < steve.arguments; i++)
		{
			fetch_argument(&j,argc, argv,&file_name);
			steve.fd[i] = inotify_init1(IN_NONBLOCK);
			steve.current_f = i;
			steve.current_w = 0;
			if( steve.fd[i] == -1 )
			{
				exit(EXIT_FAILURE);
			}
			steve.wd[i][0] = inotify_add_watch(steve.fd[i], file_name, mask);
			steve.w_count[i] = 1;
		}
	}
	else if(options[OPT_T])
	{
		for(i=0,j=1; i < steve.arguments; i++)
		{
			fetch_argument(&j,argc, argv,&file_name);
			steve.fd[i] = inotify_init1(IN_NONBLOCK);
			steve.current_f = i;
			steve.current_w = 0;
			if( steve.fd[i] == -1 )
			{
				//free(fd);
				exit(EXIT_FAILURE);
				
			}
			if( nftw(file_name,walker,20, FTW_PHYS | FTW_MOUNT) == -1)
			{
			//	free(fd);
				exit(EXIT_FAILURE);
			}
		}	
	}
	
//Poll Section - 2 sections-> 0 for Don't Add 1 for add
	for(i = 0; i < steve.arguments; i++)
	{
		poll_fd[i].fd = steve.fd[i];
		poll_fd[i].events = POLLIN;
	}
	while(1)
	{
		poll_num = poll(poll_fd,steve.arguments, -1);
		if(poll_num == -1)
		{
			if(errno == EINTR)
			{
				continue;
			}
			fprintf(stderr,"\nFailure to Begin Polling");
			exit(EXIT_FAILURE);
		}
		if (poll_num > 0)
		{
			for(j = 0; j < steve.arguments; j++)
			{
				if(poll_fd[j].revents & POLLIN)
				{
                    handle_events(steve.fd[j],steve.wd[j]);
				}
			}
		}
	}

	//free(fd);
	return EXIT_SUCCESS;
}

static void
handle_events(int fd, int *wd )
{
    /* Some systems cannot read integer variables if they are not
     * properly aligned. On other systems, incorrect alignment may
     * decrease performance. Hence, the buffer used for reading from
     * the inotify file descriptor should have the same alignment as
     * struct inotify_event. */
    char buf[4096]
    __attribute__ ((aligned(__alignof__(struct inotify_event))));
    const struct inotify_event *event;
    int i;
    ssize_t len;
    char *ptr;
    const char *mask_ptr = NULL;
    const char *type_ptr = NULL;
    char *directory = NULL;
    char buffer[80];
    time_t current_time;
    struct tm * string_time = NULL;
    
    current_time = time(&current_time);
    string_time = localtime(&current_time);
    strftime(buffer,80,TIME_OUTPUT,string_time);
    /* Loop while events can be read from inotify file descriptor. */
    for (;;)
    {
        
        /* Read some events. */
        
        len = read(fd, buf, sizeof buf);
        if (len == -1 && errno != EAGAIN)
        {
            perror("read");
            exit(EXIT_FAILURE);
        }
        
        /* If the nonblocking read() found no events to read, then
         * it returns -1 with errno set to EAGAIN. In that case,
         * we exit the loop. */
        if (len <= 0)
            break;
        
        /* Loop over all events in the buffer */
        for (ptr = buf; ptr < buf + len;
             ptr += sizeof(struct inotify_event) + event->len)
        {
            //fprintf(stdout,"Time: %s",ctime(&current_time));
            event = (const struct inotify_event *) ptr;
            
            /* Print event type */
            if (IN_CREATE & event->mask)
            {
                mask_ptr = in_create_cstring;
            }
            else if (IN_ACCESS & event->mask)
            {
                mask_ptr = in_access_cstring;
            }
            else if(IN_MODIFY & event->mask)
            {
                mask_ptr = in_modify_cstring;
            }
            else if (IN_ATTRIB & event->mask)
            {
                mask_ptr = in_attrib_cstring;
            }
            else if (IN_CLOSE_WRITE & event->mask)
            {
                mask_ptr = in_close_write_cstring;
            }
            else if (IN_CLOSE_NOWRITE & event->mask)
            {
                mask_ptr = in_close_no_write_cstring;
            }
            else if (IN_OPEN & event->mask)
            {
                mask_ptr = in_open_cstring;
            }
            else if (IN_MOVED_FROM & event->mask)
            {
                mask_ptr = in_moved_from_cstring;
            }
            else if (IN_MOVED_TO & event->mask)
            {
                mask_ptr = in_moved_to_cstring;
            }
            else if (IN_DELETE & event->mask)
            {
                mask_ptr = in_delete_cstring;
            }
            else if (IN_DELETE_SELF & event->mask)
            {
                mask_ptr = in_delete_self_cstring;
            }
            /* Once DELETE_SELF -> quit
             */
            /* Print the name of the watched directory */
            /*
            for (i = 0; i < steve.arguments; ++i)
            {
                if (wd[i] == event->wd)
                {
                    directory = safe_array[i-1];
                    break;
                }
            }
            */
            /* Print the name of the file */
            
            //            if (event->len)
            //                fprintf(stdout, "%s", event->name);
            /* Print type of filesystem object */
            
            if (event->mask & IN_ISDIR)
            {
                type_ptr = folder_cstring;
            }
            else
            {
                type_ptr = file_cstring;
            }
            //print_json(buffer,mask_ptr,directory,event->name,type_ptr);
            print_json(buffer,mask_ptr,"CANDY\0",event->name,type_ptr);
        }
    }
    fflush(stdout);
}

/* Takes two c strings and copies source into destination string
 * inserting JSON escape sequences to make it JSON safe
 */
static void json_safe(const char * source, char * destination, int size)
/*****************************************************************************
 * Function:
 * Description:
 * Parameters:
 * Pre-Conditions:
 * Post-Conditions:
 ****************************************************************************/
{
    int l = 0;
    int k = 0;
    for( ; k< size && l < PATH_LIMIT; k++)
    {
        if(source[k] == '"' || source[k] == '\n' || source[k] == '\'' || source[k] == '\t')
        {
            destination[l++] = '\\';
            destination[l++] = source[k];
        }
        else
        {
            destination[l++] = source[k];
        }
    }
    /* Just to be sure it's null terminated */
    if(l < PATH_LIMIT)
    {
        destination[l] = '\0';
    }
    else
    {
        destination[l - 1] = '\0';
    }
}
              
/* Print a JSON object as defined by JSON_OBJECT
 * Takes c style strings only, takes JSON safe directory name
 * Takes unsafe file name and makes it safe.
 */
              
/*****************************************************************************
 * Function:
 * Description:
 * Parameters:
 * Pre-Conditions:
 * Post-Conditions:
 ****************************************************************************/
static void print_json(const char * date, const char * event,const char *directory,
                const char *name, const char *type)
{
    char safe_name[NAME_MAX *2 +1];
    
    json_safe(name,safe_name,strlen(name));
    
    fprintf(stdout,JSON_OBJECT,date,event,directory,safe_name,type);
}
              
/*************************************************************************
 * Function: fetch_argument
 * Description: Takes a dependent i and all of command line arguments
 * 		purpose is to return the next command in the command line
 * 		that is not an argument
 * Parameters: Pointer i for index, command line and string address for 
 * 	       return
 * Pre-Conditions: i is exists, char exists
 * Post-Conditions: i is incremented and returns with new argument
 *************************************************************************/
static void fetch_argument(int *i, int argc, char *argv[], char **voltar)
{
    for((*i)++;*i < argc; (*i)++)
    {
        if(argv[*i][0] != '-')
        {
            *voltar = argv[*i];
            return;
        }
    }
    *voltar = NULL;
    return;
}

/*****************************************************************************
 * Function: 
 * Description:
 * Parameters: 
 * Pre-Conditions:
 * Post-Conditions: 
 ****************************************************************************/
static nfds_t count_arguments(int argc, char *argv[])
{
    int i = 1;
    nfds_t count = 0;
    for(; i < argc; i++)
    {
        if(argv[i][0] != '-')
        {
            count++;
        }
    }
    return count;
}

            
/*************************************************************************
 * Function: 	walker
 * Description: This the function called by the tree walker
 * Parameters:  NFTW provides all the information and variables
 * Pre-Conditions: None
 * Post-Conditions: NFTW function needs 0 to keep on running
*************************************************************************/
static int watch_this(const char *pathname)
{
    int *temp = NULL;
    int fd;
    
    if(steve.w_count[steve.current_f] <= steve.current_w)
    {
        temp = realloc(steve.wd[steve.current_f],
                       ((steve.w_count[steve.current_f] + INCREMENT)*sizeof(int)));
        
        if(temp != NULL)
        {
            steve.wd[steve.current_f] = temp;
            steve.w_count[steve.current_f] += 10;
        }
        else
        {
            return -1;
        }
    }
    fd = inotify_add_watch(steve.fd[steve.current_f], pathname, mask);
    if(fd == -1)
    {
        return -1;
    }
    steve.wd[steve.current_f][steve.current_w] = fd;
    steve.current_w++;
    
    return 0;
}

/*************************************************************************
 * Function: 	walker
 * Description: This the function called by the tree walker
 * Parameters:  NFTW provides all the information and variables
 * Pre-Conditions: None
 * Post-Conditions: NFTW function needs 0 to keep on running
*************************************************************************/
static int walker(const char *pathname, const struct stat *sbuf,
		int type,struct FTW *ftwb)
{
	if(sbuf->st_mode & FTW_D)
	{
        	return watch_this(pathname);
	}
	else if(sbuf->st_mode & FTW_DNR)
	{
		fprintf(stderr,"\nCan't Read: %s",pathname);
        	return -1;
	}
}
/*************************************************************************
 * Function: 	help_menu
 * Description: Prints out commands for help
 * Parameters:  verbose for debugging
 * Pre-Conditions: None
 * Post-Conditions: Prints help
*************************************************************************/
static void help_menu(char * file)
{
	fprintf(stderr,"\nGeneral: %s [w or t] [a] [n or e] [directories to be watched]",file);
	fprintf(stderr,"\n\t-w Watch Only, do NOT Traverse");
	fprintf(stderr,"\n\t-t Traverse and Add Child Directories to Watch List");
	fprintf(stderr,"\n\t-h Display this Help Menu");
	fprintf(stderr,"\n\t-a Add Created Child Directories to Watch List");
	fprintf(stderr,"\n\t-n Watch for Only Creation Events [DEFAULT]");
	fprintf(stderr,"\n\t-e Watch for ALL Events\n");
	return;
}

#define _XOPEN_SOURCE 600 //Needed to compile without Errors
#include <assert.h>
#include <errno.h>
#include <ftw.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
//#include <stdint.h>
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

static void help_menu(char* file);
static int walker(const char *pathname, const struct stat *sbuf, int type,struct FTW *ftwb);
//static int add_watcher(int fd, const char *pathname, unit32_t mask);
int main(int argc, char* argv[])
{
	int i; 
	int j;
	int poll_num;
	char *file_name;

	nfds_t nfds;
	short options[5] = {0,0,0,1,0};
	struct pollfd fds[2];
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


	//free(fd);
	return EXIT_SUCCESS;
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
	int * temp = NULL;
	if(sbuf->st_mode & FTW_D)
	{
		fprintf(stderr,"\nFound: %s",pathname);
		if(steve.w_count[steve.current_f] <= steve.current_w)
		{
			fprintf(stderr,"\nReallocating Memory");
			temp = realloc(steve.wd[steve.current_f],
				((steve.w_count[steve.current_f] + 10)*sizeof(int)));
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
		steve.wd[steve.current_f][steve.current_w] = 
			inotify_add_watch(steve.fd[steve.current_f], pathname, mask);	
		
		steve.current_w++;
		assert(steve.current_w > 0);
	}
	else if(sbuf->st_mode & FTW_DNR)
	{
		fprintf(stderr,"\nCan't Read: %s",pathname);
	}
	return 0;
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

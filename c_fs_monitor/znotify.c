#define _XOPEN_SOURCE 600 //Needed to compile without Errors
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

static void help_menu(char* file);
static int walker(const char *pathname, const struct stat *sbuf, int type,struct FTW *ftwb);

int main(int argc, char* argv[])
{
	int i; 
	int j;
	nfds_t count;
	int mask;
	int *fd;//Going to change
	int poll_num;
	int wd; //Going to change
	char *file_name;

	nfds_t nfds;
	short options[5] = {0,0,0,1,0};
	struct pollfd fds[2];

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
	count = count_arguments(argc,argv);
	fd = calloc(count, sizeof(int));
	
	
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
/*		fd = inotify_init1(IN_NONBLOCK);
		if( fd == -1 )
		{
			fprintf(stderr,"Inotify Error: %s",strerror(errno));
			exit(EXIT_FAILURE);
		}		
*/
		for(i=0,j=1; i < count; i++)
		{
		}
	}
	else if(options[OPT_T])
	{
		for(i=0,j=1; i < count; i++)
		{
			fetch_argument(&j,argc, argv,&file_name);
			fd[i] = inotify_init1(IN_NONBLOCK);
			if( fd[i] == -1 )
			{
				free(fd);
				exit(EXIT_FAILURE);
				
			}
			if( nftw(file_name,walker,20, FTW_PHYS | FTW_MOUNT) == -1)
			{
				free(fd);
				exit(EXIT_FAILURE);
			}
		}	
	}
	
//Poll Section - 2 sections-> 0 for Don't Add 1 for add


	free(fd);
	return EXIT_SUCCESS;
}

/*************************************************************************
 * Function: 	walker
 * Description: This the function called by the tree walker
 * Parameters:  NFTW provides all the information and variables
 * Pre-Conditions: None
 * Post-Conditions: NFTW function needs 0 to keep on running
*************************************************************************/
static int walker(const char *pathname, const struct stat *sbuf, int type,struct FTW *ftwb)
{
	if(sbuf->st_mode & FTW_D)
	{
		fprintf(stdout,"\nFound: %s",pathname);
		//Add to Watch List
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

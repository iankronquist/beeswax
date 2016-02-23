#include <errno.h>
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

int main(int argc, char* argv[])
{
	int i;
	short options[5] = {0,0,0,1,0};
	
	while( (i = getopt(argc, argv, "w:t:han")) != -1)
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

	return EXIT_SUCCESS;
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

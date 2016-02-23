/*
 *
 *
 *
 */ 
#ifndef __ZNOTIFY_H_
#define __ZNOTIFY_H_

#include <limits.h>

/* This is to be put a fprintf statement with 5 strings. The path is 
 * represented by two strings, directory + file or directory name
 */ 
#define JSON_OBJECT "{\"DATE\":\"%s\",\"EVENT\":\"%s\",\"PATH\":\"%s%s\",\"TYPE\":\"%s\"}\n"
#define PATH_LIMIT PATH_MAX * 2 + 1
#define TIME_OUTPUT "%c"
#define MAX_INOTIFY_INSTANCES 128 // in proc/sys/fs/inotify/max_user_*
#define MAX_WATCHES 8192
/* Takes two c strings and copies source into destination string
 * inserting JSON escape sequences to make it JSON safe
 */ 
void json_safe(const char * source, char * destination, int size)
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
void print_json(const char * date, const char * event,const char *directory, const char *name, const char *type)
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
void fetch_argument(int *i, int argc, char *argv[], char **voltar)
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
nfds_t count_arguments(int argc, char *argv[])
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
#endif

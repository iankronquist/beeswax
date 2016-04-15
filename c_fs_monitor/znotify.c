#define _XOPEN_SOURCE 600 //Needed to compile without Errors
#include <assert.h>
#include <errno.h>
#include <ftw.h>
#include <poll.h>
#include <signal.h>
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
struct pollfd *poll_fd;
int mask;

static void signal_handler(int signo);
static void cleanup();
static void handle_events(int fd, int *wd, int add_child);
static void json_safe(const char * source, char * destination, int size);
static void print_json(const char * date, const char * event,
                       const char *directory,const char *name,
                       const char *type);
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
    if(argc < 2)
    {
        fprintf(stderr,"Not Enough Arguments\n");
        help_menu(argv[0]);
        exit(EXIT_SUCCESS);
    }
    
    while( (i = getopt(argc, argv, "hanewt")) != -1)
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
                options[OPT_N] = 0;
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
                fprintf(stderr,"\nInvalid Choice: %s -h",
                        argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    steve.arguments = count_arguments(argc,argv);
    steve.fd = calloc(steve.arguments, sizeof(int));
    steve.w_count = calloc(steve.arguments,sizeof(int));
    steve.w_last = calloc(steve.arguments,sizeof(int));
    steve.wd = calloc(steve.arguments, sizeof(int*));
    steve.path = calloc(steve.arguments, sizeof(char *));
    
    poll_fd = calloc(steve.arguments,sizeof(struct pollfd));
    if(options[OPT_T])
    {
        for(i = 0; i < steve.arguments; i++)
        {
            steve.w_count[i] = DEFAULT_WATCH_NUMBER;
            steve.wd[i] = calloc(DEFAULT_WATCH_NUMBER,sizeof(int));
            steve.path[i] = calloc(DEFAULT_WATCH_NUMBER,
                                   sizeof(char *));
        }
    }
    else if(options[OPT_W])
    {
        for(i = 0; i < steve.arguments; i++)
        {
            steve.w_count[i] = 1;
            steve.wd[i] = calloc(1,sizeof(int));
            steve.path[i] = calloc(1,sizeof(char *));
            steve.path[i][0] = calloc(PATH_LIMIT,sizeof(char));
        }
    }
    
    atexit(cleanup);
    signal(SIGINT,signal_handler);
    
    if(options[OPT_N]) //Report Only New Directories
    {
        mask = IN_CREATE;
    }
    else if(options[OPT_E]) //Report all events
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
            steve.wd[i][0] =
            inotify_add_watch(steve.fd[i], file_name, mask);
            json_safe(file_name,steve.path[i][0],strlen(file_name));
            steve.w_last[i] = 1;
            steve.w_count[i] = 1;
            status(steve);
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
                exit(EXIT_FAILURE);
                
            }
            if( nftw(file_name,walker,20,
                     FTW_MOUNT | FTW_DEPTH) == -1)
            {
                exit(EXIT_FAILURE);
            }
        }
    }
    else
    {
        exit(EXIT_FAILURE);
    }
    //Poll Section
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
                    steve.current_f = j;
                    handle_events(steve.fd[j],
                                  steve.wd[j],
                                  options[OPT_A]);
                }
            }
        }
    }
    
    return EXIT_SUCCESS;
}

/*****************************************************************************
 * Function: 	   signal_handler
 * Description:	   Catches SIGINT->defined in main
 * Parameters:     Signal Number
 * Pre-Conditions: Control+C must be entered
 * Post-Conditions:Program ends after calling cleanup, defined in main
 ****************************************************************************/
static void signal_handler(int signo)
{
    exit(EXIT_SUCCESS);
}
/*****************************************************************************
 * Function: 	   cleanup
 * Description:    Frees all memory used by the structure znotify
 *		   Note: Used to help with valgrind
 * Parameters:     None
 * Pre-Conditions: Exit Called
 * Post-Conditions:Heap Memory is freed
 ****************************************************************************/
static void cleanup()
{
    int i;
    int j;
    
    for( i = 0; i < steve.arguments; i++)
    {
        free(steve.wd[i]);
        for(j = 0; j < steve.w_last[i]; j++)
        {
            free(steve.path[i][j]);
        }
        free(steve.path[i]);
    }
    free(poll_fd);
    free(steve.path);
    free(steve.w_last);
    free(steve.wd);
    free(steve.fd);
    free(steve.w_count);
}

/*****************************************************************************
 * Function: 	   handle_events
 * Description:    Generic Event Handler, Reads from file discriptor for
 *		   events and sends information to print
 * Parameters:     File Desciptor and WatchDescriptor pointer
 * Pre-Conditions: Event Occured
 * Post-Conditions:Event is Reported
 ****************************************************************************/
static void
handle_events(int fd, int *wd,int add_child )
{
    /* Some systems cannot read integer variables if they are not
     * properly aligned. On other systems, incorrect alignment may
     * decrease performance. Hence, the buffer used for reading from
     * the inotify file descriptor should have the same alignment as
     * struct inotify_event. */
    char buf[4096]
    __attribute__ ((aligned(__alignof__(struct inotify_event))));
    const struct inotify_event *event;
    char pathname_buffer[PATH_LIMIT];
    int i;
    int length;
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
            event = (const struct inotify_event *) ptr;
            
            for (i = 0; i < steve.w_last[steve.current_f]; ++i)
            {
                if (wd[i] == event->wd)
                {
                    steve.current_w = i;
                    break;
                }
            }
            
            //Identify if Folder or File
            if (event->mask & IN_ISDIR)
            {
                type_ptr = folder_cstring;
            }
            else
            {
                type_ptr = file_cstring;
            }
            /* Print event type */
            if (IN_CREATE & event->mask)
            {
                mask_ptr = in_create_cstring;
                if(add_child && (strcmp(type_ptr,folder_cstring)==0))
                {
                    strcpy(pathname_buffer,  steve.path[steve.current_f]
                           [steve.current_w]);
                    length = strlen(pathname_buffer);
                    if(pathname_buffer[length - 1] != '/')
                    {
                        pathname_buffer[length] = '/';
                        pathname_buffer[length + 1] = '\0';
                    }
                    strcat(pathname_buffer,event->name);
                    //watch_this relies on current_w
                    steve.current_w = steve.w_last[steve.current_f];
                    if(watch_this(pathname_buffer) == -1)
                    {
                        fprintf(stderr,"Failed to add: %s\n",
                                pathname_buffer);
                    }
                    //watch_this will increment current_w
                    steve.current_w = i;
                    status(steve);
                }
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
            
            if(event->len == 0)
            {
                print_json(buffer,mask_ptr,
                           steve.path[steve.current_f][steve.current_w],
                           "",type_ptr);
            }
            else
            {
                print_json(buffer,mask_ptr,
                           steve.path[steve.current_f][steve.current_w],
                           event->name,type_ptr);
            }
        }
    }
    fflush(stdout);
}

/*****************************************************************************
 * Function:	   json_safe
 * Description:    Makes pathnames safe to be printed out as JSON
 * Parameters:     2 c strings, and length of source
 * Pre-Conditions: None
 * Post-Conditions:Destination is now Safe to be printed
 ****************************************************************************/
static void json_safe(const char * source, char * destination, int size)
{
    int l = 0;
    int k = 0;
    
    for( ; k< size && l < PATH_LIMIT; k++)
    {
        if(source[k] == '"' || source[k] == '\n' || source[k] == '\'' || source[k] == '\t' || source[k] == '\\')
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

/*****************************************************************************
 * Function:	   print_json
 * Description:	   Prints to Standard Output JSON object defined in
 *	  	   znotify.h
 * Parameters:	   Date, type of event, pathanme, file or directory
 * Pre-Conditions: Assumes directory is properly escaped for JSON
 * Post-Conditions:Prints string to standard output
 ****************************************************************************/
static void print_json(const char * date, const char * event,
                       const char *directory, const char *name, const char *type)
{
    char *safe_name = NULL;
    if (name[0] != 0)
    {
        safe_name = calloc((NAME_MAX * 2 + 1),sizeof(char));
        json_safe(name,safe_name,strlen(name));
        fprintf(stdout,JSON_OBJECT,date,event,directory,safe_name,type);
        free(safe_name);
    }
    else
    {
        //name will be \0
        fprintf(stdout,JSON_OBJECT,date,event,directory,name,type);
    }
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
 * Function: 	   count_arguments
 * Description:	   Counts arguments in the command line that do not begin
 *		   with '-'
 * Parameters: 	   Number of arguments, string of arguments
 * Pre-Conditions: None
 * Post-Conditions:Return count of arguments without a dash at the beginning
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
 * Function: 	   watch_this
 * Description:    Adds an inotify instance to a file descriptor.
 * Parameters:     A pathname
 * Pre-Conditions: Needs the znotify struct, the current_w and current_f
 *		   needs to be defined
 * Post-Conditions:Additional inotify instance with the w_last increased
 *	           If additional space needs to increase, w_count will
 *		   increase be INCREMENT as defined in znotify.h. The
 *		   pathname will be entered into the path variable
 *************************************************************************/
static int watch_this(const char *pathname)
{
    int *temp = NULL;
    char **temp_paths = NULL;
    int fd;
    
    if(steve.w_count[steve.current_f] <= steve.w_last[steve.current_f])
    {
        temp = realloc(steve.wd[steve.current_f],
                       ((steve.w_count[steve.current_f] + INCREMENT)*sizeof(int)));
        temp_paths = realloc(steve.path[steve.current_f],
                             ((steve.w_count[steve.current_f] + INCREMENT)*sizeof(char*)));
        if((temp != NULL) && temp_paths != NULL)
        {
            steve.wd[steve.current_f] = temp;
            steve.path[steve.current_f] = temp_paths;
            steve.w_count[steve.current_f] += INCREMENT;
        }
        else
        {
            return -1;
        }
        
    }
    fd = inotify_add_watch(steve.fd[steve.current_f], pathname, mask);
    if(fd == -1)
    {
        if((errno == ENOSPC) &&
           (steve.w_count[steve.current_f] >= MAX_WATCHES))
        {
            fprintf(stderr,"Error: Try Putting Subdirectories"
                    " in Command Line\n");
        }
        else
        {
            fprintf(stderr,"Error Occured Adding to Watch List: %s %s\n",
                    pathname,strerror(errno));
        }
        return -1;
    }
    steve.wd[steve.current_f][steve.current_w] = fd;
    // Get JSON safe pathname
    steve.path[steve.current_f][steve.current_w] =
				calloc(PATH_LIMIT,sizeof(char));
    json_safe(pathname,
              steve.path[steve.current_f][steve.current_w],
              strlen(pathname));
    steve.w_last[steve.current_f] += 1;
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
    if(sbuf->st_mode & S_IFDIR)
    {
        return watch_this(pathname);
    }
    else if(sbuf->st_mode & FTW_DNR)
    {
        fprintf(stderr,"Can't Read: %s\n",pathname);
        return 0;
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
    fprintf(stderr,"General: %s [w or t] [a] [n or e]"
            "[directories to be watched]",file);
    fprintf(stderr,"\n\t-w Watch Only, do NOT Traverse");
    fprintf(stderr,"\n\t-t Traverse and Add Child Directories to Watch List");
    fprintf(stderr,"\n\t-h Display this Help Menu");
    fprintf(stderr,"\n\t-a Add Created Child Directories to Watch List");
    fprintf(stderr,"\n\t-n Watch for Only Creation Events [DEFAULT]");
    fprintf(stderr,"\n\t-e Watch for ALL Events\n");
    return;
}

/*************************************************************************
 * Function: 	status
 * Description: Prints out the struct for debugging
 * Parameters:  verbose for debugging
 * Pre-Conditions: None
 * Post-Conditions: Prints the pathname and other information
 *************************************************************************/
static void status(struct znotify z)
{
    int i;
    int j;
    
    fprintf(stderr,"\tNumber of Arguments: %d\n",z.arguments);
    fprintf(stderr,"\tcurrent_f: %d current_w: %d\n",z.current_f,z.current_w);
    for(i=0; i < z.arguments; i++)
    {
        fprintf(stderr,"\tFile Descriptor: %d\n",z.fd[i]);
        fprintf(stderr,"\tWatch Descriptor Count: %d out of %d\n",z.w_last[i],z.w_count[i]);
        for(j=0; j<z.w_last[i];j++)
        {
            fprintf(stderr,"\tPathname[%d][%d] %s\n",i,j,z.path[i][j]);
        }
    }
}
#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <time.h>
#include <limits.h>

#define DEBUG 1
#define JSON_OBJECT "{\"DATE\":\"%s\",\"EVENT\":\"%s\",\"PATH\":\"%s\",\"TYPE\":\"%s\"}"
//Maximimum pathname, all escapes and 1 null terminating
#define PATH_LIMIT PATH_MAX * 2 + 1 

//Defined in limits.h
const char one[]    = "IN_ACCESS";
const char two[]    = "IN_MODIFY";
const char four[]   = "IN_ATTRIB";
const char eight[]  = "IN_CLOSE_WRITE";
const char ten[]    = "IN_CLOSE_NOWRITE";
const char twenty[] = "IN_OPEN";
const char fourty[] = "IN_MOVED_FROM"
const char eighty[] = "IN_MOVED_TO";
const char hundo[]  = "IN_CREATE";
const char thundo[] = "IN_DELETE";
const char fundo[]  = "IN_DELETE_SELF";

/*
 *
 */
static void print_json(const char *name)
{
	char name_buffer[ PATH_LIMIT ];
	name_buffer[0] = '\0';

	fprintf(stdout,JSON_OBJECT,"date","event",name,"type");
}


/* Read all available inotify events from the file descriptor 'fd'.
 *          wd is the table of watch descriptors for the directories in argv.
 *                     argc is the length of wd and argv.
 *                               argv is the list of watched directories.
 *                                         Entry 0 of wd and argv is unused. */

static void
handle_events(int fd, int *wd, int argc, char* argv[])
{
    /* Some systems cannot read integer variables if they are not
     *               properly aligned. On other systems, incorrect alignment may
     *                             decrease performance. Hence, the buffer used for reading from
     *                                           the inotify file descriptor should have the same alignment as
     *                                                         struct inotify_event. */
    
    char buf[4096]
    __attribute__ ((aligned(__alignof__(struct inotify_event))));
    const struct inotify_event *event;
    int i;
    ssize_t len;
    char *ptr;
    const char *mask_ptr = NULL;
    time_t current_time;
    current_time = time(&current_time);
    
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
         *                   it returns -1 with errno set to EAGAIN. In that case,
         *                                     we exit the loop. */
        
        if (len <= 0)
            break;
        
        /* Loop over all events in the buffer */
        
        for (ptr = buf; ptr < buf + len;
             ptr += sizeof(struct inotify_event) + event->len)
        {
            fprintf(stdout,"Time: %s",ctime(&current_time));
            event = (const struct inotify_event *) ptr;
            
            /* Print event type */
            //Fix this section
            if (IN_ACCESS & event->mask)
            {
                mask_ptr = one;
            }
            else if(IN_MODIFY & event->mask)
            {
                fprintf(stdout,"IN_MODIFY: ");
            }
            else if (IN_ATTRIB & event->mask)
            {
                fprintf(stdout,"IN_ATTRIB: ");
            }
            else if (IN_CLOSE_WRITE & event->mask)
            {
                fprintf(stdout,"IN_CLOSE_WRITE: ");
            }
            else if (IN_CLOSE_NOWRITE & event->mask)
            {
                fprintf(stdout,"IN_CLOSE_NOWRITE: ");
            }
            else if (IN_OPEN & event->mask)
            {
                fprintf(stdout,"IN_OPEN: ");
            }
            else if (IN_MOVED_FROM & event->mask)
            {
                fprintf(stdout,"IN_MOVED_FROM: ");
            }
            else if (IN_MOVED_TO & event->mask)
            {
                fprintf(stdout,"IN_MOVED_TO: ");
            }
            else if (IN_CREATE & event->mask)
            {
                fprintf(stdout,"IN_CREATE: ");
            }
            else if (IN_DELETE & event->mask)
            {
                fprintf(stdout,"IN_DELETE: ");
            }
            else if (IN_DELETE_SELF & event->mask)
            {
                fprintf(stdout,"IN_DELETE_SELF: ");
            }
            /*if (IN_CLOSE & event->mask)
            {
                fprintf(stdout,"IN_CLOSE: ");
            }
            if (IN_MOVE & event->mask)
            {
                fprintf(stdout,"IN_MOVE: ");
            }*/
/*            if(DEBUG)
            {
                fprintf(stdout, "UMASK: %X", event->mask);
            }
  */      
	    /* Once DELETE_SELF -> quit
	    */
            /* Print the name of the watched directory */
            
            for (i = 1; i < argc; ++i)
            {
                if (wd[i] == event->wd)
                {
                    fprintf(stdout,"%s/", argv[i]);
                    break;
                }
            }
            
            /* Print the name of the file */
            
            if (event->len)
                fprintf(stdout, "%s", event->name);
            /* Print type of filesystem object */
            
            if (event->mask & IN_ISDIR)
                fprintf(stdout, " [directory] \n");
            else
                fprintf(stdout, " [file] \n");
            

            print_json(event->name);
        }
    }
}

int
main(int argc, char* argv[])
{
    char buf;
    int fd, i, poll_num;
    int *wd;
    nfds_t nfds;
    struct pollfd fds[2];
    
    if (argc < 2)
    {
        fprintf(stdout, "Usage: %s PATH [PATH ...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    fprintf(stdout, "Press ENTER key to terminate.\n");
    
    /* Create the file descriptor for accessing the inotify API */
    
    fd = inotify_init1(IN_NONBLOCK);
    if (fd == -1)
    {
        perror("inotify_init1");
        exit(EXIT_FAILURE);
    }
    
    /* Allocate memory for watch descriptors */
    
    wd = calloc(argc, sizeof(int));
    if (wd == NULL)
    {
        perror("calloc");
        exit(EXIT_FAILURE);
    }
    
    /* Mark directories for events
     *               - file was opened
     *                             - file was closed */
    
    for (i = 1; i < argc; i++)
    {
        wd[i] = inotify_add_watch(fd, argv[i],
                                  IN_ALL_EVENTS);
        if (wd[i] == -1)
        {
            fprintf(stderr, "Cannot watch '%s'\n", argv[i]);
            perror("inotify_add_watch");
            exit(EXIT_FAILURE);
        }
    }
    
    /* Prepare for polling */
    
    nfds = 2;
    
    /* Console input */
    
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    
    /* Inotify input */
    
    fds[1].fd = fd;
    fds[1].events = POLLIN;
    
    /* Wait for events and/or terminal input */
    
    fprintf(stdout, "Listening for events.\n");
    while (1)
    {
        poll_num = poll(fds, nfds, -1);
        if (poll_num == -1)
        {
            if (errno == EINTR)
                continue;
            perror("poll");
            exit(EXIT_FAILURE);
        }
        
        if (poll_num > 0)
        {
            
            if (fds[0].revents & POLLIN)
            {
                
                /* Console input is available. Empty stdin and quit */
                
                while (read(STDIN_FILENO, &buf, 1) > 0 && buf != '\n')
                    continue;
                break;
            }
            
            if (fds[1].revents & POLLIN)
            {
                
                /* Inotify events are available */
                
                handle_events(fd, wd, argc, argv);
            }
        }
    }
    
    fprintf(stdout, "Listening for events stopped.\n");
    
    /* Close inotify file descriptor */
    
    close(fd);
    
    free(wd);
    exit(EXIT_SUCCESS);
}

#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <time.h>

#include "znotify.h"

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




/* Read all available inotify events from the file descriptor 'fd'.
 * wd is the table of watch descriptors for the directories in argv.
 * argc is the length of wd and argv.
 * argv is the list of watched directories.
 * Entry 0 of wd and argv is unused. */
static void
handle_events(int fd, int *wd, int argc,char *safe_array[])
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
            // FIXME: Fix this section
            if (IN_ACCESS & event->mask)
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
            else if (IN_CREATE & event->mask)
            {
                mask_ptr = in_create_cstring;
            }
            else if (IN_DELETE & event->mask)
            {
                mask_ptr = in_delete_cstring;
            }
            else if (IN_DELETE_SELF & event->mask)
            {
                mask_ptr = in_delete_self_cstring;
            }
            /*if (IN_CLOSE & event->mask)
             {
             fprintf(stdout,"IN_CLOSE: ");
             }
             if (IN_MOVE & event->mask)
             {
             fprintf(stdout,"IN_MOVE: ");
             }*/
            /* Once DELETE_SELF -> quit
             */
            /* Print the name of the watched directory */
            
            for (i = 0; i < argc; ++i)
            {
                if (wd[i] == event->wd)
                {
                    directory = safe_array[i-1];
                    break;
                }
            }
            
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
            print_json(buffer,mask_ptr,directory,event->name,type_ptr);
        }
    }
    fflush(stdout);
}

int
main(int argc, char* argv[])
{
    char **safe_array;
    int fd, i,j, poll_num;
    int *wd;
    nfds_t nfds;
    struct pollfd fds[2];
    
    if (argc < 2)
    {
        fprintf(stdout, "Usage: %s PATH [PATH ...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    
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
     * - file was opened
     * - file was closed */
    for (i = 1; i < argc; i++)
    {
        wd[i] = inotify_add_watch(fd, argv[i],
                                  IN_ALL_EVENTS| IN_ONLYDIR);
        if (wd[i] == -1)
        {
            fprintf(stderr, "Cannot watch '%s'\n", argv[i]);
            perror("inotify_add_watch");
            exit(EXIT_FAILURE);
        }
    }
    
    /* Prepare for polling */
    
    nfds = 1;
    
    /* Inotify input */
    
    fds[0].fd = fd;
    fds[0].events = POLLIN;
    
    /* Creating JSON safe directory */
    safe_array = malloc((argc) * sizeof(char*));
    for(i = 0,j = 1; i < argc - 1; i++,j++)
    {
        safe_array[i] = malloc(PATH_LIMIT * sizeof(char));
        json_safe(argv[j],safe_array[i],strlen(argv[j]));
    }
    
    /* Wait for events and/or terminal input */
    fprintf(stderr, "Listening for events.\n");
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
                
                /* Inotify events are available */
                handle_events(fd, wd, argc, safe_array);
            }
        }
    }
    
    fprintf(stdout, "Listening for events stopped.\n");
    
    /* Close inotify file descriptor */
    close(fd);
    
    free(wd);
    for(i = 0; i < argc; i++)
    {
        free(safe_array[i]);
    }
    free(safe_array);
    
    exit(EXIT_SUCCESS);
}

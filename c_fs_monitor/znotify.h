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
#define JSON_OBJECT "{\"DATE\":\"%s\",\"EVENT\":\"%s\",\"PATH\":\"%s/%s\",\"TYPE\":\"%s\"}\n"
#define PATH_LIMIT (PATH_MAX * 2 + 1)
#define TIME_OUTPUT "%s"
#define MAX_INOTIFY_INSTANCES 128 // in proc/sys/fs/inotify/max_user_*
#define MAX_WATCHES 8192
#define DEFAULT_WATCH_NUMBER 2 
#define INCREMENT 10

#define C_ACCESS        "IN_ACCESS"
#define C_MODIFY        "IN_MODIFY"
#define C_ATTRIB        "IN_ATTRIB"
#define C_CLOSE_WRITE   "IN_CLOSE_WRITE"
#define C_CLOSE_NOWRITE "IN_CLOSE_NOWRITE"
#define C_OPEN          "IN_OPEN"
#define C_MOVED_FROM    "IN_MOVED_FROM"
#define C_MOVED_TO      "IN_MOVED_TO"
#define C_CREATE        "IN_CREATE"
#define C_DELETE        "IN_DELETE"
#define C_DELETE_SELF   "IN_DELETE_SELF"
#define C_UNMOUNT       "IN_UNMOUNT"
#define C_Q_OVERFLOW    "IN_Q_OVERFLOW"
#define C_IGNORED       "IN_IGNORED"
#define C_UNKNOWN       "Unknown"

#define C_FOLDER         "FOLDER"
#define C_FILE           "FILE"

struct znotify
{
	int arguments; //Command Line directories
	int *fd;      
	int **wd;
	int current_f; 
	int current_w;
	int *w_count; //Keep track of max number of watch descriptors
	int *w_last;  //Keep track of last watch descriptor in use
	char ***path;
};

static void status(struct znotify z);
#endif

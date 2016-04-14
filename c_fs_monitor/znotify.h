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


#endif

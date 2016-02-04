/*
    This is a simple C program which is a stub for the FS monitor.
    It takes one argument which would be a directory to monitor. In this case,
    the filename is discarded after argument validation.
    Every minute the 
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main (int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Not enough arguments");
        exit(EXIT_FAILURE);
    }

    while (1) {
        fprintf(stdout, "{ \"key\": \"value\" }\n");
        fflush(stdout);
        fprintf(stderr, "tick\n");
        sleep(1);
    }
}


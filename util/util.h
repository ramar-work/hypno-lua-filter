#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>


#ifndef UTIL_H
#define UTIL_H
unsigned char *read_file ( const char *filename, int *len, char *err, int errlen ) ;
#endif

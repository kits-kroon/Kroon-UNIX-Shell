#ifndef _UTILITIES_H
#define _UTILITIES_H

// open()
#include <unistd.h>
#include <fcntl.h>
// not technically required, but needed on some UNIX distributions for open() //
#include <sys/types.h>
#include <sys/stat.h>
// isspace()
#include <ctype.h>
// strlen()
#include <string.h>
// printf()
#include <stdio.h>
// exit()
#include <stdlib.h>

int fileInRead(char *fileName);

int fileOutCreate(char *fileName);

int checkWhiteSpace(char *str);

char *trim(char *str); // trims leading and trailing whitespace

#endif
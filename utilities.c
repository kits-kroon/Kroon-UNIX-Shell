#include "utilities.h"

int fileOutCreate(char *fileName) {
    int fd_out;

    if ((fd_out = open(fileName, O_WRONLY | O_TRUNC | O_CREAT,
                       S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)) < 0) {
        printf("file %s could not be opened or created\n", fileName);
        fflush(stdout);
        exit(1);
    }

    return fd_out;
}

int fileInRead(char *fileName) {
    int fd_in;

    if ((fd_in = open(fileName, O_RDONLY)) < 0) {
        printf("file %s could not be opened\n", fileName);
        fflush(stdout);
        exit(1);
    }

    return fd_in;
}

int checkWhiteSpace(char *str) {
    int i = 0;

    for (i = 0; i < strlen(str); i++) {
        if (str[i] != ' ' && str[i] != '\t')
            return 1;
    }

    return 0;
}

/* code acquired from Stack Overflow via user: PedroCunha_67677's comment
   made on the 19th of April, 2014.
   http://stackoverflow.com/questions/23154867/segmentation-faultcore-dumped-structure
   THIS IS NOT FOR DATA USING MEMORY ALLOCATIONS, THIS WILL CAUSE MEMORY LEAKS IN THOSE
   CASES.                                                                            */
char *trim(char *str)
{
    char *end; // will represent the final character in the string

    while(isspace(*str)) // continues whilst a space exists at the index being checked.
        str++;           // increments str to the next character

    if(*str == '\0')     // returns true if the end has been reached. i.e. the null pointer
        return str;      // returns the string unchanged.

    end = str + strlen(str) - 1; /* This makes end equal to string pointer and then 
                                    adds the string length to it excluding the NULL
                                    pointer */

    while(end > str && isspace(*end)) // checks for spaces, at the end and decrements
        end--;                        // the index to be the final character of the
                                      // actual string (no whitespace)

    *(end + 1) = '\0'; // appends a null terminator to the string again 
                       // at the end where no white space exists.
    
    return str; // returns str
}
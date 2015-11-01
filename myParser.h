#ifndef _MYPARSER_H
#define _MYPARSER_H

/*
 * myParser.h
 * Provides the command data structure and tools
 * for parsing.
 * 
 * parser.h and parser.c were used as a base for this toolkit,
 * these were authored by Michael Roberts and provided to the ICT310 class
 * by Hong Xie with the intention of providing a base to be built from.
 * 
 * Author: Jake Kroon
 */

#include <stdio.h>     // fprintf()
#include <stdlib.h>    // exit()
#include <string.h>    // strdup(), strchr(), strtok(), strstr(), 
#include <glob.h>      // glob(), globfree(), glob_t
#include "utilities.h" // checkWhiteSpace(), trim()

/* Length of command line */
#define CMD_LENGTH 131072 /* 128MB this is the same command buffer size as UBUNTU 15.10
                            and will satisfy project requirements */

/* whitespaces that are searched for */
static const char whiteSpace[3] = {(char) 0x20, (char) 0x09, (char) 0x00};

/* Structure for commands */
typedef struct Command_struct {

	// path name of the command
    char *com_pathname;

    // number of arguments to the command
    int argc;

	// pointer to strings, each string
    // is an argument for the command, including
    // argument "0". the last pointer should be
    // set to NULL
    char **argv;

    // if this is not NULL
 	// std input is redirected to given filename
    char *redirect_in;

    // if this is not NULL
   	// std output is redirected to given filename
   	// is redirected to the given filename
    char *redirect_out;

	// ' ' = no command suffix or sequential(last command)
    // '&' the command is followed by '&'
    // ';' the command is followed by ';'
    // '|' the command is followed by '|'
    char com_suffix;
					   
} command;

command **process_cmd_line(char *cmd, int);

void process_cmd(char *cmd, command *result);

void process_simple_cmd(char *cmd, command *result);

void printComStruct(struct Command_struct *com);

void clean_up(command **cmd);

void setEmptyComSuffix(command ***cmd_line, char **cmd, int *lc);

void setSemiColComSuffix(command ***cmd_line, char **cmd, char **rc, int *lc);

void setBarComSuffix(command ***cmd_line, char **cmd, char **rc, int *lc);

void setAmpersandComSuffix(command ***cmd_line, char **cmd, char **rc, int *lc);

#endif
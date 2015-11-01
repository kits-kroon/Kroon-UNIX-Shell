#ifndef _SHELL_H
#define _SHELL_H

#include "myParser.h"  // command, process_cmd_line(), clean_up(), CMD_LENGTH
#include "utilities.h" // checkWhiteSpace(), 
#include <stdlib.h>    // exit(), getenv()
#include <stdio.h> 	   // printf(), fgets(), fflush()
#include <string.h>	   // strtok(), strcmp(), strcat(), strlen()
#include <signal.h>	   // signal()
#include <unistd.h>	   // pipe(), fork(), close(), dup2(), getcwd()
					   // STDIN_FILENO, STDOUT_FILENO, chdir(), 
#include <fcntl.h>	   // dup2(), open()
#include <sys/types.h> // pid_t, open(), waitpid()
#include <sys/stat.h>  // open()
#include <sys/wait.h>  // waitpid()
#include <ctype.h>
#include <errno.h>	  // errno, EINTR

void printPrompt();
void printWelcome();
void executePipes(command ***cl, int lc);
int  handleMyCommands(command ****cl, int lc);
int  handleMyForkables(command ****cl, int lc);
void handleProcessInput(command ****cl, int p[][2], int i, int in);
void handleProcessOutput(command ****cl, int p[][2], int i, int out, int lc);
void exitCommand();
void promptCommand(command *****cl, int lc);
void pwdCommand();
void cdCommand(command *****cl, int lc);
void childSignalHandler();

char *prompt; // prompt for shell

#endif
#include "myParser.h"

void process_simple_cmd(char *cmd, command* result)
{
	char *dc;
	int lpc = 1;

	if(checkWhiteSpace(cmd) == 0){
		result -> com_pathname = strdup("BAD_ENTRY_ERROR");
		result -> argv = realloc((void *) result -> argv, sizeof(char *));
		result -> argv[0] = strdup("BAD_ENTRY_ERROR");
		result -> argc = 0;
	}
	else
	{
		/*No spaces means no arguments */
		if (((dc = strchr(cmd, whiteSpace[0])) == NULL
			&& (dc = strchr(cmd, whiteSpace[1])) == NULL)
			&& (dc = strchr(cmd, whiteSpace[2])) == NULL)
		{
			result -> com_pathname = strdup(cmd);
			result -> argv = realloc((void *) result -> argv, sizeof(char *));
			result -> argv[0] = strdup(cmd);
			result -> argc = 0;
		}

		/* pull out the command name (first token) */
		dc = strtok(cmd, whiteSpace);
		
		result -> com_pathname = strdup(dc);
		result -> argv = realloc((void *) result -> argv, sizeof(char *));
		result -> argv[0] = strdup(dc);
		result -> argc++;

		/* loop through the remaining tokens, writing them to the struct */
		while ((dc = strtok(NULL, whiteSpace)) != NULL)
		{
			// insert glob here!
			if((strstr(dc, "*") != NULL) || strstr(dc, "?") != NULL)
			{
				glob_t gbuf;
				gbuf.gl_offs = 512; // any more than this is obscene surely?
				glob(dc, 0, NULL, &gbuf);

				int i = 0;
				for(i = 0; i < gbuf.gl_pathc; i++)
				{
					result -> argv = realloc((void *) result-> argv, (lpc + 1) * sizeof(char *));
					result -> argv[lpc] = strdup(gbuf.gl_pathv[i]);
					result -> argc++;
					lpc++;
				}

				globfree(&gbuf); // frees memory allocated by glob

			}
			else
			{
				result -> argv = realloc((void *) result -> argv, (lpc + 1) * sizeof(char *));
				result -> argv[lpc] = strdup(dc);
				result -> argc++;
				lpc++;
			}
		}
	}

	// set the final array element to NULL
	result -> argv = realloc((void *) result -> argv, (lpc + 1) * sizeof(char *));
	result -> argv[lpc] = NULL;

	return;
}

void process_cmd(char *cmd, command *result)
{
	char *pc, *mc;
	char *simple_cmd = NULL;

	/* if no redirection is found, then only a simple command is present */
	if ((pc = strchr(cmd, '<')) == NULL)
	{
		if((pc = strchr(cmd, '>')) == NULL)
		{
			process_simple_cmd(cmd, result);
			result -> redirect_in = NULL;
			result -> redirect_out = NULL;
		}
		else /* output redirection exists */
		{
			pc = strtok(cmd, ">");
			simple_cmd = strdup(pc);

			pc = strtok(NULL, "\0");

			process_simple_cmd(simple_cmd, result);

			pc = trim(pc);
			result -> redirect_out = strdup(pc);
		}
	} 
	else /* input redirection exists */
	{
		pc = strtok(cmd, "<");
		simple_cmd = strdup(pc);
		pc = strtok(NULL, "\0");

		// check again for output redirection
		if((mc = strchr(simple_cmd, '>')) != NULL)
			process_cmd(simple_cmd, result);
		if((mc = strchr(pc, '>')) != NULL)
			process_cmd(pc, result); 

		process_simple_cmd(simple_cmd, result);

		pc = trim(pc);
		result -> redirect_in = strdup(pc);
	}

	free(simple_cmd);
	return;
}

command** process_cmd_line(char *cmd, int new)
{
	char *ac; // ampersand character 
	char *bc; // bar character
	char *sc; // semicolon character
	static command **cmd_line;
	static int lc;

	// ensures statics are null when not called recursively.
	if(new == 1){
		lc = 0;
		cmd_line = NULL;
	}

	/* Check for the existence of delimiters
	 * If none exist, we only have one command on the command line.
	 * otherwise process accordingly.
	 */

	ac = strchr(cmd, '&');
	bc = strchr(cmd, '|');
	sc = strchr(cmd, ';');

	if (ac == NULL &&
	 	bc == NULL &&
	 	sc == NULL)
	{
		setEmptyComSuffix(&cmd_line, &cmd, &lc);
	}

	if (ac != NULL && bc != NULL && sc != NULL)
	{
		if ((ac - cmd + 1) < (bc - cmd + 1)
		&& (ac - cmd + 1) < (sc - cmd + 1))
		{
			setAmpersandComSuffix(&cmd_line, &cmd, &ac, &lc);
		}
		else if ((bc - cmd + 1)  < (sc - cmd + 1)
			&&  (bc - cmd + 1)  < (ac - cmd + 1))
		{
			setBarComSuffix(&cmd_line, &cmd, &bc, &lc);
		}
		else
		{
			setSemiColComSuffix(&cmd_line, &cmd, &sc, &lc);
		}
	}
	else if(ac != NULL && bc != NULL)
	{
		if((ac - cmd + 1) < (bc - cmd + 1))
		{
			setAmpersandComSuffix(&cmd_line, &cmd, &ac, &lc);
		}
		else
		{
			setBarComSuffix(&cmd_line, &cmd, &bc, &lc);
		}
		
	}
	else if(ac != NULL && sc != NULL)
	{
		if((ac - cmd + 1) < (sc - cmd + 1))
		{
			setAmpersandComSuffix(&cmd_line, &cmd, &ac, &lc);
		}
		else
		{
			setSemiColComSuffix(&cmd_line, &cmd, &sc, &lc);
		}
		
	}
	else if(sc != NULL && bc != NULL)
	{
		if((sc - cmd + 1) < (bc - cmd + 1))
		{
			setSemiColComSuffix(&cmd_line, &cmd, &sc, &lc);
		}
		else
		{
			setBarComSuffix(&cmd_line, &cmd, &bc, &lc);
		}
	}
	else if(ac != NULL)
	{
		setAmpersandComSuffix(&cmd_line, &cmd, &ac, &lc);
		
	}
	else if(bc != NULL)
	{
		setBarComSuffix(&cmd_line, &cmd, &bc, &lc);
	}
	else if(sc != NULL)
	{
		setSemiColComSuffix(&cmd_line, &cmd, &sc, &lc);
	}

	 cmd_line = realloc((void *) cmd_line, (lc + 1) * sizeof(command *));
	 cmd_line[lc] = NULL;

	 return cmd_line;
}

void clean_up(command **cmd)
{
    int lpc = 0;
    int ilpc = 0;

    while(cmd[lpc] != NULL){      
      ilpc = 0;

      cmd[lpc] -> argc = 0;

      if(cmd[lpc] -> com_pathname != NULL){
          free(cmd[lpc] -> com_pathname);
      }

      if(cmd[lpc] -> argv != NULL){
          while(cmd[lpc] -> argv[ilpc] != NULL)
          {
              free(cmd[lpc] -> argv[ilpc]);
              ilpc++;
          }
          free(cmd[lpc] -> argv);
      }

      if(cmd[lpc] -> redirect_in != NULL)
        free(cmd[lpc] -> redirect_in);

      if(cmd[lpc] -> redirect_out != NULL)
        free(cmd[lpc] -> redirect_out);

      free(cmd[lpc]);
      lpc++;
    }

    free(cmd);
   	cmd = NULL;

    return;
}

void printComStruct(struct Command_struct *com)
{ 
    int i;

    fprintf(stderr,"com_pathname=%s\n", com->com_pathname); 
    fprintf(stderr,"argc=%d\n", com->argc); 
    for(i=0;  com->argv[i]!=NULL; i++) 
        fprintf(stderr,"argv[%d]=%s\n", i, com->argv[i]); 
        fprintf(stderr,"#######\n"); 
        if (com->redirect_in == NULL)
            fprintf(stderr,"redirect_in=NULL\n"); 
        else
            fprintf(stderr,"redirect_in=%s\n", com->redirect_in); 
        if (com->redirect_out == NULL)
            fprintf(stderr,"redirect_out=NULL\n"); 
        else
            fprintf(stderr,"redirect_out=%s\n", com->redirect_out); 
        fprintf(stderr,"com_suffix=%c\n\n", com->com_suffix); 
}

void setAmpersandComSuffix(command ***cmd_line, char **cmd, char **rc, int *lc)
{
 	(*rc) = strtok((*cmd), "&"); // a "&" was found
 	(*rc) = strtok(NULL, ""); // get second token out

 	(*cmd_line) = realloc((void *) (*cmd_line), ((*lc) + 1) * sizeof(command *));
 	(*cmd_line)[(*lc)] = calloc(1, sizeof(command));

 	process_cmd((*cmd), (*cmd_line)[(*lc)]);
 	(*cmd_line)[(*lc)] -> com_suffix = '&';
 	(*lc)++;

 	if((*rc) != NULL)
 		process_cmd_line((*rc), 0); // process second token	
}

void setBarComSuffix(command ***cmd_line, char **cmd, char **rc, int *lc)
{
	(*rc) = strtok((*cmd), "|"); // a "|" was found
	(*rc) = strtok(NULL, "");  // get second token out

	(*cmd_line) = realloc((void *) (*cmd_line), ((*lc) + 1) * sizeof(command *));
	(*cmd_line)[(*lc)] = calloc(1, sizeof(command));

 	process_cmd((*cmd), (*cmd_line)[(*lc)]);
	(*cmd_line)[(*lc)] -> com_suffix = '|';
	(*lc)++;

	if((*rc) != NULL)
		process_cmd_line((*rc), 0); // process second token	
}

void setSemiColComSuffix(command ***cmd_line, char **cmd, char **rc, int *lc)
{
	(*rc) = strtok((*cmd), ";"); // a ';' was found
	(*rc) = strtok(NULL, "");  // get second token out

	(*cmd_line) = realloc((void *) (*cmd_line), ((*lc) + 1) * sizeof(command *));
	(*cmd_line)[(*lc)] = calloc(1, sizeof(command));

	process_cmd((*cmd), (*cmd_line)[(*lc)]);
	(*cmd_line)[(*lc)] -> com_suffix = ';';
	(*lc)++;

	if((*rc) != NULL)
		process_cmd_line((*rc), 0); // process second token
}

void setEmptyComSuffix(command ***cmd_line, char **cmd, int *lc)
{
	(*cmd_line) = realloc((*cmd_line), ((*lc) + 1) * sizeof(command *));

	if((*cmd_line) == NULL)
		exit(-1);

	(*cmd_line)[(*lc)] = calloc(1, sizeof(command));

	if((*cmd_line)[(*lc)] == NULL)
		exit(-1);

	(*cmd_line)[(*lc)] -> com_pathname = NULL;
	(*cmd_line)[(*lc)] -> argc = 0;
	(*cmd_line)[(*lc)] -> argv = NULL;
	(*cmd_line)[(*lc)] -> redirect_in = NULL;
	(*cmd_line)[(*lc)] -> redirect_out = NULL;
	(*cmd_line)[(*lc)] -> com_suffix = ' ';

	process_cmd((*cmd), (*cmd_line)[(*lc)]);

	(*lc)++;	
}
#include "shell.h"

#define PROMPT_BUFFER 256

int main(void) {
    char *cmd;
 	command **cl;
	struct sigaction sa;

    prompt = (char *) malloc(sizeof(char) * 2);
    prompt[0] = '%';
    prompt[1] = '\0';

    sa.sa_handler = &childSignalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if(sigaction(SIGCHLD, &sa, 0) == -1) {
    	perror("Child signal error\n");
    	exit(1);
    }

    // ignore signals
    signal(SIGTSTP, SIG_IGN); // CTRL-Z if this is moved it stops functioning
    signal(SIGINT, SIG_IGN);  // CTRL-C
    signal(SIGQUIT, SIG_IGN); // CTRL backslash

    cmd = (char *) malloc(CMD_LENGTH);

    printWelcome();

    while (1) {
        int lc = 0;
        int again = 1;

        printPrompt();

        while (again) {
            again = 0;
            fgets(cmd, CMD_LENGTH, stdin);
            if (cmd == NULL) if (errno == EINTR)
                again = 1; // signal interruption, read again..
        }

        // removes all /n unless the string is only
        strtok(cmd, "\n");

        if (checkWhiteSpace(cmd) == 1 && cmd[0] != '\n') // checks for ' ' and '\t'
        {
            cl = process_cmd_line(cmd, 1);

            while (cl[lc] != NULL) {
                lc++;
            }

            executePipes(&cl, lc);
            clean_up(cl);  // free command struct memory
        }
    }

    free(cmd);
}

void printPrompt() {
    printf("%s ", prompt);
}

int handleMyCommands(command ****cl, int lc) {
    if (strcmp("exit", (**cl)[lc]->argv[0]) == 0) {
        exitCommand();
    }
    else if (strcmp("cd", (**cl)[lc]->argv[0]) == 0) {
        cdCommand(&cl, lc);
        return 1;
    }
    else if (strcmp("prompt", (**cl)[lc]->argv[0]) == 0) {
        promptCommand(&cl, lc);
        return 1;
    }

    return 0;
}

int handleMyForkables(command ****cl, int lc) {
    if (strcmp("pwd", (**cl)[lc]->argv[0]) == 0) {
        pwdCommand();
        return 1;
    }

    return 0;
}

void executePipes(command ***cl, int lc) {
    int i;
    pid_t pid;
    int p[lc][2];
    int countBackground = 0;

    for (i = 0; i < lc; i++)
        pipe(p[i]); // inefficient and really could use some work

    for (i = 0; i < lc; i++)
    {
        if (handleMyCommands(&cl, i) == 1)
            continue;

        if ((pid = fork()) < 0) {
            printf("fork error");
            fflush(stdout);
        }

        int in = p[i][0];
        int out = p[i][1];

        if (pid == 0) {
           // signal(SIGCHLD, childSignalHandler); // ensures the child stays dead

            /* The following signal changes are non-functional */
            /* They have been left however for further review  */
            // signal(SIGINT, SIG_DFL);  // CTRL-C reactivates for child
            // signal(SIGTSTP, SIG_DFL); // CTRL-Z reactivates for child

            handleProcessInput(&cl, p, i, in);
            handleProcessOutput(&cl, p, i, out, lc);

            close(in);
            if ((*cl)[i]->com_suffix != '|')
                close(out);


            if (handleMyForkables(&cl, i) == 0) {
                execvp((*cl)[i]->argv[0], (*cl)[i]->argv); // checks for file
                execv((*cl)[i]->argv[0], (*cl)[i]->argv);  // checks for full path
                printf("Command %s could not be found or bad arguments\n", (*cl)[i]->argv[0]);
                fflush(stdout);
                exit(1);
            }
            else {
                exit(0);
            }
        }
        else {
            close(out);
            if ((*cl)[i]->com_suffix != '&') {
                waitpid(pid, (int *) 0, 0);
            }
            else {
                countBackground++;
                printf("[%d] %d\n", countBackground, pid);
            }
        }
    }
}


void handleProcessOutput(command ****cl, int p[][2], int i, int out, int lc) {
	if ((**cl)[i]->redirect_out == NULL) {
	    if (i != (lc - 1) && (**cl)[i]->com_suffix == '|') {
	        fflush(stdout);
	        dup2(out, STDOUT_FILENO);
	    }
	    else if ((**cl)[i]->redirect_in == NULL &&
	             (**cl)[i]->com_suffix == '|') {
	        fflush(stdout);
	        dup2(p[i - 1][1], STDOUT_FILENO);
	    }
	}
	else {
	    out = fileOutCreate((**cl)[i]->redirect_out);
	    dup2(out, STDOUT_FILENO);
	}
}

void handleProcessInput(command ****cl, int p[][2], int i, int in) {
    if (i != 0) {
        fflush(stdin);
        if ((**cl)[i]->redirect_in == NULL &&
            (**cl)[i - 1]->com_suffix == '|') {
            dup2(p[i - 1][0], STDIN_FILENO);
        }
        else if ((**cl)[i]->redirect_in != NULL) {
            in = fileInRead((**cl)[i]->redirect_in);
            dup2(in, STDIN_FILENO);
        }
    }
    else {
        fflush(stdin);
        if ((**cl)[i]->redirect_in != NULL) {
            in = fileInRead((**cl)[i]->redirect_in);
            dup2(in, STDIN_FILENO);
        }
    }
}

void printWelcome() {
    printf("*******************************************************\n");
    printf("\t Kroon Shell version 1.0\n");
    printf("\t Author: Jake Kroon\n");
    printf("\t ICT310 Final Project: A Simple UNIX Shell\n");
    printf("*******************************************************\n");
}

void exitCommand() {
    printf("bye now!\n");
    exit(0);
}

void promptCommand(command *****cl, int lc) {
    if ((***cl)[lc]->argc > 1) {
        int i = 0;
        char buf[PROMPT_BUFFER] = "";

        for (i = 1; i < (***cl)[lc]->argc; i++) {
            strcat(buf, (***cl)[lc]->argv[i]);
        }

        if ((int) strlen(buf) <= PROMPT_BUFFER) {
            free(prompt);
            prompt = (char *) malloc(sizeof(char) * ((int) strlen(buf) + 1));
            for (i = 0; i < strlen(buf); i++) {
                prompt[i] = buf[i];
            }

            prompt[i] = '\0';
        }
        else {
            printf("prompt error: too many characters\n");
        }
    }
    else {
        printf("prompt error: too few arguments\n");
    }

}

void pwdCommand() {
    char cwd[1024];

    getcwd(cwd, sizeof(cwd));
    printf("%s\n", cwd);

}

void cdCommand(command *****cl, int lc) {
    if ((***cl)[lc]->argc == 1) {
        chdir(getenv("HOME"));
    }
    else {
        int i = 0;
        char buf[PROMPT_BUFFER] = "";

        for (i = 1; i < (***cl)[lc]->argc; i++) {
            strcat(buf, (***cl)[lc]->argv[i]);
            if(i < ((***cl)[lc]->argc - 1))
            	strcat(buf, " ");
        }

        if ((chdir(buf) == -1))
            printf("cd error: directory not found, %s\n", buf);
    }
}

void childSignalHandler() {
    int more = 1; // more zombies to claim
    pid_t pid;
    int status;

    while (more) {
        pid = waitpid(-1, &status, WNOHANG);
        if (pid <= 0)
            more = 0;
    }
}
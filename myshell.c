#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

char *readline (const char *prompt);

typedef struct {
    char **args;
} cmd;

// parse cmd from command line input
cmd *cmd_init(cmd *cmd, char *line) {
    int numArgs = 0;
    char *arg;
    cmd->args = malloc(sizeof(char *) * (numArgs + 1));
    while ((arg = strsep(&line, " ")) != NULL) {
        char *emptyString = "";
        if (strcmp(arg, emptyString) != 0) {
            cmd->args[numArgs] = arg;
            numArgs ++;
            cmd->args = realloc(cmd->args, sizeof(char *) * (numArgs + 1));
        }
    }
    cmd->args[numArgs] = 0;
    // free after finish using ....
    // free(emptyString);
    free(arg);
    return cmd;
}

// execute cmd
void cmd_exec(cmd *cmd) {
    char *command = cmd->args[0];
    execvp(command, cmd->args);
}

int main(int argc, char **argv) {
    while (1) {
        char *line, *token;
        line = readline("myshell>");
        while ((token = strsep(&line, ";")) != NULL) {
            int childStatus;
            pid_t pid;
            pid = fork();
            if (pid == -1) {
                // fork error
            }
            else if (pid == 0) {
                // child process
                cmd *cmd;
                cmd_init(cmd, token);
                cmd_exec(cmd);
            }
            else {
                // parent process
                waitpid(pid, &childStatus, WUNTRACED);
            }
        }
        // free *line and *token after finish using
        free(token);
        free(line);
    }
}

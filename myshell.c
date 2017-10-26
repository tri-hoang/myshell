#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

char *readline (const char *prompt);

typedef struct {
    char **args;
} cmd;

void cmd_exec(cmd *cmd) {
    execvp(cmd->args[0], cmd->args);
}

cmd *cmd_init(char *string) {
    int numArg = 0;
    char *arg;
    cmd *cmd;
    cmd->args = malloc(sizeof(char *) * (numArg + 1));
    while ((arg = strsep(&string, " ")) != NULL) {
        // might need fixing ..
        char *emptyString = "";
        if (strcmp(arg, emptyString) != 0) {
            cmd->args[numArg] = arg;
            numArg ++;
            cmd->args = realloc(cmd->args, sizeof(char * ) * (numArg + 1));
        }
    }
    cmd->args[numArg] = NULL;
    return cmd;
}

int main(int argc, char **argv) {
    while(1) {
        char *lineInput = (char *)NULL;
        char *token;
        lineInput = readline("myshell>");
        while ((token = strsep(&lineInput, ";")) != NULL) {
            int childStatus;
            pid_t pid;
            pid = fork();
            if (pid == -1) {
                // fork error
            }
            else if (pid == 0) {
                cmd *cmd;
                cmd = cmd_init(token);
                cmd_exec(cmd);
            }
            else {
                waitpid(pid, &childStatus, WUNTRACED);
            }
        }
        // set to NULL and free after finish using..
        lineInput = NULL;
        token = NULL;
        free(lineInput);
        free(token);
    }
}

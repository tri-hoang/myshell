#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

char *readline (const char *prompt);

typedef struct {
    char **args;
    char *input;
    char *output;
} cmd;

void cmd_exec(cmd *cmd) {
    execvp(cmd->args[0], cmd->args);
}

cmd *cmd_init(char *lineInput) {
    char *cmdArgs;
    if ((cmdArgs = malloc(strlen(lineInput) + 1)) == NULL) {
        fprintf(stderr, "malloc fails");
    }
    strcpy(cmdArgs, lineInput);
    int numArg = 0;
    cmd *cmd;
    cmd->args = malloc(sizeof(char *) * (numArg + 1));
    const char o_redirection[2] = ">";
    const char i_redirection[2] = "<";
    const char e_redirection[3] = "2>";
    const char o1_redirection[3] = "1>";
    if (strstr(cmdArgs, o_redirection) != NULL && strstr(cmdArgs, i_redirection) != NULL) {
        size_t o_length = strlen(strstr(cmdArgs, o_redirection));
        size_t i_length = strlen(strstr(cmdArgs, i_redirection));
        if (o_length > i_length) {
            if (strstr(cmdArgs, o1_redirection) != NULL) {
                cmdArgs = strsep(&cmdArgs, o1_redirection);
            }
            else if (strstr(cmdArgs, e_redirection) != NULL) { 
                cmdArgs = strsep(&cmdArgs, e_redirection);
            }
            else {
                cmdArgs = strsep(&cmdArgs, o_redirection);
            }
            strsep(&lineInput, o_redirection);
            char *temp = strsep(&lineInput, o_redirection);
            char *cpyString = malloc(strlen(temp) + 1);
            strcpy(cpyString, temp);
            cmd->output = strsep(&temp, i_redirection);
            cmd->input = strsep(&cpyString, i_redirection);
            cmd->input = strsep(&cpyString, i_redirection);
        }
        else {
            cmdArgs = strsep(&cmdArgs, i_redirection);
            strsep(&lineInput, i_redirection);
            char *temp =  strsep(&lineInput, i_redirection);
            char *cpyString = malloc(strlen(temp) + 1);
            strcpy(cpyString, temp);
            if (strstr(temp, o1_redirection) != NULL) {
                cmd->input = strsep(&temp, o1_redirection);
            }
            else if (strstr(temp, e_redirection) != NULL) {
                cmd->input = strsep(&temp, e_redirection);
            }
            else {
                cmd->input = strsep(&temp, o_redirection);
            }
            cmd->output = strsep(&cpyString, o_redirection);
            cmd->output = strsep(&cpyString, o_redirection);
        }
    }
    else if (strstr(cmdArgs, o_redirection) != NULL) {
        cmd->input = NULL;
        if (strstr(cmdArgs, o1_redirection) != NULL) {
            cmdArgs = strsep(&cmdArgs, o1_redirection);
        }
        else if (strstr(cmdArgs, e_redirection) != NULL) { 
            cmdArgs = strsep(&cmdArgs, e_redirection);
        }
        else {
            cmdArgs = strsep(&cmdArgs, o_redirection);
        }
        strsep(&lineInput, o_redirection);
        cmd->output = strsep(&lineInput, o_redirection);
    }
    else if (strstr(cmdArgs, i_redirection) != NULL) {
        cmd->output = NULL;
        cmdArgs = strsep(&cmdArgs, i_redirection);
        cmd->input = strsep(&lineInput, i_redirection);
        cmd->input = strsep(&lineInput, i_redirection);
    }
    else {
        cmd->output = NULL;
        cmd->input = NULL;
    }
    printf("\nCHECKING HERE: ...\n");
    printf("cmdArg is : %s\ncmd->output is : %s\ncmd->input is : %s\n", cmdArgs, cmd->output, cmd->input);
    
    char *arg;
    while ((arg = strsep(&cmdArgs, " ")) != NULL) {
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
        lineInput = NULL;
        token = NULL;
        free(lineInput);
        free(token);
    }
}

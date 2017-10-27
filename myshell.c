#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

char *readline (const char *prompt);
char *strip_whitespace(char *string);
char **parse_input_command(char *string); 

typedef struct {
    char **array;
    int size;
} cmd_array;

typedef struct {
    char **args; // deprecated
    char **cmd_array;
    int size;
    char *input;
    char *output;
    char *error;
} cmd;

// execute cmd
// should use a loop to loop through cmd->cmd_array
// usage should be sth like: 
//      string = "this is | a pipe < input redir > output redir"
//      cmd = cmd_init(string)
//      ->  cmd->cmd_array = [["this is"], ["a pipe"]]
//          cmd->size = size(cmd->cmd_array) = 2
//          cmd->input = input redir
//          cmd-> output = output redir
//          cmd-> error = NULL (not specified = NULL)
//      cmd_exec(cmd) ...
//          for x in cmd->cmd_array:
//              parse_input_command(x)
//              execvp(x[0], x)
void cmd_exec(cmd *cmd) {
    // parse_input_command will parse an array of string -> an array of string to be called by execvp
    // i.e: input: [["   ls -a "], ["ps"],["cat file "]]
    //      output: [["ls","-a",NULL],["ps", NULL], ["cat", "file", NULL]]     
    char **test = parse_input_command(cmd->cmd_array[0]);
    execvp(test[0], test);
    // test -> function works .. need to add more to this (y)
    //execvp(cmd->args[0], cmd->args);
}

// helper function for parsing "|"
// input: "ls -a | pipe | to | somewhere | else"
// output: [["ls -a "], [" pipe "], [" to ]", [" somewhere "] , [" else "]]
cmd_array process_pipe(char *lineInput) {
    const char pipe_symbol[2] = "|";
    char *copy_input;
    if ((copy_input = malloc(strlen(lineInput) + 1)) == NULL) {
        fprintf(stderr, "malloc fails");
    }
    strcpy(copy_input, lineInput);
    cmd_array cmd_array;
    cmd_array.size = 0;
    cmd_array.array = malloc(sizeof(char *) * (cmd_array.size + 1));
    char *cmd_string;
    while ((cmd_string = strsep(&copy_input, "|")) != NULL) {
        char *emptyString = "";
        if (strcmp(cmd_string, emptyString) != 0) {
            cmd_array.array[cmd_array.size] = cmd_string;
            cmd_array.size ++;
            cmd_array.array = realloc(cmd_array.array, sizeof(char *) * (cmd_array.size + 1));
        }
    }
    return cmd_array;
}

// take a string input from user's command line input
// initialize cmd struct based on given string: 
// - cmd-> cmd_array is an array of commands, separated by "|"
//      i.e: "pipe1 abc | pipe2 abc| pipe3 abc < input redir > output redir" 
//      then cmd->cmd_array = [["pipe1 abc"], ["pipe2 abc"], ["pipe3 abc"]]
// - cmd->size is the length of cmd->cmd_array
// - cmd->input is the path to redirect input (if none provided cmd->input = NULL)
// - cmd->output is the path to redirect output (if none provided cmd->output = NULL)
// - cmd->error is the path to redirect error (if none provided cmd->error = NULL)
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
    const char oe_redirection[3] = "&>";
    int same_oe = 0;
    // quick fix
    int *error_redir = malloc(sizeof(int));
    *error_redir = 0;
    // assume user will use only one of the four ">", "1>", "&>", "2>"
    // if there are both "<" and "?>" in the command line input
    if (strstr(cmdArgs, o_redirection) != NULL && strstr(cmdArgs, i_redirection) != NULL) {
        size_t o_length = strlen(strstr(cmdArgs, o_redirection));
        size_t i_length = strlen(strstr(cmdArgs, i_redirection));
        // "?>" occurs before "<"
        if (o_length > i_length) {
            if (strstr(cmdArgs, o1_redirection) != NULL) {
                cmdArgs = strsep(&cmdArgs, o1_redirection);
            }
            else if (strstr(cmdArgs, oe_redirection) != NULL) {
                cmdArgs = strsep(&cmdArgs, oe_redirection);
                same_oe = 1;
            }
            else if (strstr(cmdArgs, e_redirection) != NULL) {
                cmdArgs = strsep(&cmdArgs, e_redirection);
                *error_redir = 1;
            }
            else {
                cmdArgs = strsep(&cmdArgs, o_redirection);
            }
            strsep(&lineInput, o_redirection);
            char *temp = strsep(&lineInput, o_redirection);
            char *cpyString = malloc(strlen(temp) + 1);
            strcpy(cpyString, temp);
            if (*error_redir == 1) {
                cmd->error = strsep(&temp, i_redirection);
                cmd->output = NULL;
            }
            else {
                cmd->output = strsep(&temp, i_redirection);
                cmd->error = NULL;
            }
            cmd->input = strsep(&cpyString, i_redirection);
            cmd->input = strsep(&cpyString, i_redirection);
            if (same_oe == 1) { cmd->error = cmd->output; }
        }
        // "<" occurs before "?>"
        else {
            cmdArgs = strsep(&cmdArgs, i_redirection);
            strsep(&lineInput, i_redirection);
            char *temp =  strsep(&lineInput, i_redirection);
            char *cpyString = malloc(strlen(temp) + 1);
            strcpy(cpyString, temp);
            if (strstr(temp, o1_redirection) != NULL) {
                cmd->input = strsep(&temp, o1_redirection);
            }
            else if (strstr(temp, oe_redirection) != NULL) {
                cmd->input = strsep(&temp, oe_redirection);
                same_oe = 1;
            }
            else if (strstr(temp, e_redirection) != NULL) {
                cmd->input = strsep(&temp, e_redirection);
                *error_redir = 1;
            }
            else {
                cmd->input = strsep(&temp, o_redirection);
            }
            if (*error_redir == 1) {
                cmd->error = strsep(&cpyString, o_redirection);
                cmd->error = strsep(&cpyString, o_redirection);
                cmd->output = NULL;
            }
            else { 
                cmd->output = strsep(&cpyString, o_redirection);
                cmd->output = strsep(&cpyString, o_redirection);
                cmd->error = NULL;
            }
            if (same_oe == 1) { cmd->error = cmd->output; }
        }
    }
    // if there is only "?>"
    else if (strstr(cmdArgs, o_redirection) != NULL) {
        cmd->input = NULL;
        if (strstr(cmdArgs, o1_redirection) != NULL) {
            cmdArgs = strsep(&cmdArgs, o1_redirection);
        }
        else if (strstr(cmdArgs, oe_redirection) != NULL) {
            cmdArgs = strsep(&cmdArgs, oe_redirection);
            same_oe = 1;
        }
        else if (strstr(cmdArgs, e_redirection) != NULL) {
            cmdArgs = strsep(&cmdArgs, e_redirection);
            *error_redir = 1;
        }
        else {
            cmdArgs = strsep(&cmdArgs, o_redirection);
        }
        strsep(&lineInput, o_redirection);
        if (*error_redir == 1) {
            cmd->error = strsep(&lineInput, o_redirection);
            cmd->output = NULL;
        }
        else {
            cmd->output = strsep(&lineInput, o_redirection);
            cmd->error = NULL;
        }
        if (same_oe == 1) { cmd->error = cmd->output; }
    }
    // if there is only "<"
    else if (strstr(cmdArgs, i_redirection) != NULL) {
        cmd->output = NULL;
        cmd->error = NULL;
        cmdArgs = strsep(&cmdArgs, i_redirection);
        cmd->input = strsep(&lineInput, i_redirection);
        cmd->input = strsep(&lineInput, i_redirection);
    }
    // if there is neither "<" nor "?>"
    else {
        cmd->output = NULL;
        cmd->input = NULL;
        cmd->error = NULL;
    }
    if (cmd->output != NULL) { cmd->output = strip_whitespace(cmd->output); }
    if (cmd->input != NULL) { cmd->input = strip_whitespace(cmd->input); }
    if (cmd->error != NULL) { cmd->error = strip_whitespace(cmd->error); }
    
    
    // replace cmd->args with cmd->cmd_array and cmd->size
    const char pipe_symbol[2] = "|";
    cmd->size = 0;
    cmd->cmd_array = malloc(sizeof(char *) * (cmd->size + 1));
    char *cmd_string;
    while ((cmd_string = strsep(&cmdArgs, pipe_symbol)) != NULL) {
        char *emptyString = "";
        if (strcmp(cmd_string, emptyString) != 0) {
            cmd->cmd_array[cmd->size] = cmd_string;
            cmd->size ++;
            cmd->cmd_array = realloc(cmd->cmd_array, sizeof(char *) * (cmd->size + 1));
        }
    }

    printf("cmd->size is : %d\ncmd->output is : %s\ncmd->input is : %s\ncmd->error is : %s\n", cmd->size, cmd->output, cmd->input, cmd->error);
    // we don't need this anymore
    //char *arg;
    //while ((arg = strsep(&cmdArgs, " ")) != NULL) {
        // might need fixing ..
    //    char *emptyString = "";
    //    if (strcmp(arg, emptyString) != 0) {
    //        cmd->args[numArg] = arg;
    //        numArg ++;
    //        cmd->args = realloc(cmd->args, sizeof(char * ) * (numArg + 1));
    //   }
    //}
    //cmd->args[numArg] = NULL;
    return cmd;
}

char **parse_input_command(char *string) {
    int size = 0;
    char **parsed_command = malloc(sizeof(char *) * (size + 1));
    char *arg;
    while ((arg = strsep(&string, " ")) != NULL) {
        char *emptyString = "";
        if (strcmp(arg, emptyString) != 0) {
            parsed_command[size] = arg;
            size ++;
            parsed_command = realloc(parsed_command, sizeof(char *) * (size + 1));
        }
    }
    parsed_command[size] = NULL;
    return parsed_command;
}   


// helper function to remove whitespace
char *strip_whitespace(char *string) {
    char *end;
    while(isspace((unsigned char)*string)) string++;
    if(*string ==0) { return string; }
    end = string + strlen(string) - 1;
    while (end > string && isspace((unsigned char)*end)) end--;
    *(end + 1) = 0;
    return string;
}

int main(int argc, char **argv) {
    while(1) {
        char *lineInput = (char *)NULL;
        char *token;
        // read input from user
        lineInput = readline("myshell>");
        // split user's input line with delimiter ";" into commands
        // i.e: given string:   this is a command ; this is another command 
        // -> output:           token = this is a command
        //                      token = this is another command
        //                      -> execute each command sequentially
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
        // free pointers and set to NULL after finish using..
        lineInput = NULL;
        token = NULL;
        free(lineInput);
        free(token);
    }
}

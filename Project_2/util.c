#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "util.h"
#include "comm.h"

char * commands[] = {"\\list", "\\kick", "\\p2p", "\\seg", "\\exit"};

int start_with(const char *pre, const char *str)
{
    return strncmp(pre, str, strlen(pre));
}

/*
 * Print shell prompt. Pass the username as argument.
 */
void print_prompt(char *name)
{
    printf("%s >> ", name);
    fflush(stdout);
}


int parse_line(char * input, char * tokens[], char * delim) {
    int i = 0;
    char *tok = strtok(input, delim);

    while(tok != NULL) {
        tokens[i] = tok;
        i++;
        tok = strtok(NULL, delim);
    }
    tokens[i] = NULL;
    return i;
}

enum command_type find_command_type(char * command) {
    int i = 0;

    for (i=0;i<EXIT+1;i++) {
        if (strcmp(command, commands[i]) == 0) {
            return i;
        }
    }

	return BROADCAST;
}

enum command_type get_command_type(char * full_command) {
	char msg[MAX_MSG];
	strcpy(msg, full_command);
	char * tokens[128];

	parse_line(msg, tokens, " ");
	return find_command_type(tokens[0]);
}




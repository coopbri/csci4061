enum command_type {LIST=0, KICK=1, P2P=2, SEG=3, EXIT=4, BROADCAST=5};

/* Checks if the given string starts with a particular substring */
int start_with(const char *pre, const char *str);

/*
 * Print shell prompt. Pass the username as argument.
 */
void print_prompt(char *name);


/* 
 * Parses the input string passed to it
 * Takes in a pointer to the array of tokens
 * returns the total number of tokens read
 */
int parse_line(char * input, char * tokens[], char * delim);

/* Returns the command type of the command passed to it */
enum command_type get_command_type(char * command);

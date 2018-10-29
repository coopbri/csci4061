#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include "comm.h"

/* -------------------------Main function for the client ----------------------*/
void main(int argc, char * argv[]) {

	int pipe_user_reading_from_server[2], pipe_user_writing_to_server[2];

	// You will need to get user name as a parameter, argv[1].

	if(connect_to_server("YOUR_UNIQUE_ID", argv[1], pipe_user_reading_from_server, pipe_user_writing_to_server) == -1) {
		exit(-1);
	}

	/* -------------- YOUR CODE STARTS HERE -----------------------------------*/

	
	// poll pipe retrieved and print it to sdiout

	// Poll stdin (input from the terminal) and send it to server (child process) via pipe

		
	/* -------------- YOUR CODE ENDS HERE -----------------------------------*/
}

/*--------------------------End of main for the client --------------------------*/



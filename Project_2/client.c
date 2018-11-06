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
#include "util.h"

/* -------------------------Main function for the client----------------------*/
void main(int argc, char *argv[]) {

  int pipe_user_reading_from_server[2], pipe_user_writing_to_server[2];

  // You will need to get user name as a parameter, argv[1].

  if (connect_to_server("YOUR_UNIQUE_ID", argv[1], pipe_user_reading_from_server,
                        pipe_user_writing_to_server) == -1) {
    exit(-1);
  }
/* -------------- YOUR CODE STARTS HERE -----------------------------------*/
  char stuff[MAX_MSG];

  close(pipe_user_writing_to_server[0]); //close reading end of this pipe
  while(1)
  {
    print_prompt(argv[1]);

    while(read(0, stuff, MAX_MSG) == -1){ usleep(50);} // non blocking read from stdin
    write(pipe_user_writing_to_server[1], stuff, strlen(stuff));
    memset(stuff, 0, sizeof(stuff)); //clear buffer

    printf("successfully wrote\n");
    usleep(40000);
  }

  // poll pipe retrieved and print it to sdiout

  // Poll stdin (input from the terminal) and send it to server (child process)
  // via pipe

  /* -------------- YOUR CODE ENDS HERE -----------------------------------*/
}

/*--------------------------End of main for the client--------------------------*/

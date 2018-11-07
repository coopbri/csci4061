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
  char feedback[MAX_MSG];

  close(pipe_user_writing_to_server[0]); //close reading end of this pipe
  print_prompt(argv[1]);
  while(1)
  {
    // non blocking read to see what server sends
    while (read(pipe_user_reading_from_server[0], feedback, MAX_MSG) > 0) {
      usleep(600);
      printf("Feedback: %s\n", feedback);
      memset(feedback, 0, sizeof(feedback)); // clear buffer
      print_prompt(argv[1]);
    }

    while(read(0, stuff, MAX_MSG) > 0){
      usleep(600);
      write(pipe_user_writing_to_server[1], stuff, strlen(stuff));
      memset(stuff, 0, sizeof(stuff)); //clear buffer
      print_prompt(argv[1]);
    } // non blocking read from stdin

    usleep(40000);
  }

  // poll pipe retrieved and print it to sdiout

  // Poll stdin (input from the terminal) and send it to server (child process)
  // via pipe

  /* -------------- YOUR CODE ENDS HERE -----------------------------------*/
}

/*--------------------------End of main for the client--------------------------*/

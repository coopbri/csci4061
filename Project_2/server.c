#include "comm.h"
#include "util.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

char *user_commands[] = {"\\list", "\\kick", "\\p2p", "\\seg", "\\exit"};

/* -----------Functions that implement server functionality-------------------------*/

/*
 * Returns the empty slot on success, or -1 on failure
 */
int find_empty_slot(USER *user_list) {
  // iterate through the user_list and check m_status to see if any slot is
  // EMPTY return the index of the empty slot
  int i = 0;
  for (i = 0; i < MAX_USER; i++) {
    if (user_list[i].m_status == SLOT_EMPTY) {
      return i;
    }
  }
  return -1;
}

/*
 * list the existing users on the server shell
 */
int list_users(int idx, USER *user_list) {
  // iterate through the user list
  // if you find any slot which is not empty, print that m_user_id
  // if every slot is empty, print "<no users>""
  // If the function is called by the server (that is, idx is -1), then printf
  // the list If the function is called by the user, then send the list to the
  // user using write() and passing m_fd_to_user return 0 on success
  int i, flag = 0;
  char buf[MAX_MSG] = {}, *s = NULL;

  /* construct a list of user names */
  s = buf;
  strncpy(s, "---connected user list---\n",
          strlen("---connected user list---\n"));
  s += strlen("---connected user list---\n");
  for (i = 0; i < MAX_USER; i++) {
    if (user_list[i].m_status == SLOT_EMPTY)
      continue;
    flag = 1;
    strncpy(s, user_list[i].m_user_id, strlen(user_list[i].m_user_id));
    s = s + strlen(user_list[i].m_user_id);
    strncpy(s, "\n", 1);
    s++;
  }
  if (flag == 0) {
    strcpy(buf, "<no users>\n");
  } else {
    s--;
    strncpy(s, "\0", 1);
  }

  if (idx < 0) {
    printf("%s",buf);
    printf("\n");
  } else {
    /* write to the given pipe fd */
    //write(user_list[idx].m_fd_write_to_child, buf, strlen(buf) + 1) < 0
    if (write(user_list[idx].m_fd_write_to_child, buf, strlen(buf) + 1) < 0)
      perror("writing to server shell");
  }

  return 0;
}

/*
 * add a new user
 */
int add_user(int idx, USER *user_list, int pid, char *user_id,
             int pipe_write_to_child, int pipe_write_to_server, int pipe_read_from_child,
             int pipe_read_from_server) {
  // populate the user_list structure with the arguments passed to this function
  strcpy(user_list[idx].m_user_id, user_id);
  user_list[idx].m_pid = pid;
  user_list[idx].m_fd_write_to_child = pipe_write_to_child;
  user_list[idx].m_fd_write_to_server = pipe_write_to_server;
  user_list[idx].m_fd_read_from_child = pipe_read_from_child;
  user_list[idx].m_fd_read_from_server = pipe_read_from_server;
  user_list[idx].m_status = 0;
  // return the index of user added
  return idx;
}

/*
 * Kill a user
 */
void kill_user(int idx, USER *user_list) {
  // kill a user (specified by idx) by using the systemcall kill()
  // then call waitpid on the user
  char kick_message[MAX_MSG] = "You have been kicked";
  write(user_list[idx].m_fd_write_to_child, kick_message, MAX_MSG);
  usleep(500000);
  int ret;
  int status;
  ret = kill(user_list[idx].m_pid, SIGKILL);
  if (ret == 0) {
    waitpid(user_list[idx].m_pid, &status, WNOHANG);
  } else {
    perror("Problem killing child");
  }

}

/*
 * Perform cleanup actions after the used has been killed
 */
void cleanup_user(int idx, USER *user_list) {
  // m_pid should be set back to -1
  user_list[idx].m_pid = -1;

  // m_user_id should be set to zero, using memset()
  memset(user_list[idx].m_user_id, 0, MAX_USER_ID);

  // close all the fd
  close(user_list[idx].m_fd_write_to_server);
  close(user_list[idx].m_fd_read_from_server);
  close(user_list[idx].m_fd_write_to_child);
  close(user_list[idx].m_fd_read_from_server);

  // set the value of all fd back to -1
  user_list[idx].m_fd_write_to_server = -1;
  user_list[idx].m_fd_read_from_server = -1;
  user_list[idx].m_fd_write_to_child = -1;
  user_list[idx].m_fd_read_from_server = -1;

  // set the status back to empty
  user_list[idx].m_status = SLOT_EMPTY;
}

/*
 * Kills the user and performs cleanup
 */
void kick_user(int idx, USER *user_list) {

  kill_user(idx, user_list);
  cleanup_user(idx, user_list);
}

/*
 * broadcast message to all users
 */
int broadcast_msg(USER *user_list, char *buf, char *sender) {
  // iterate over the user_list and if a slot is full, and the user is not the
  // sender itself, then send the message to that user return zero on success
  for(int i = 0; i < MAX_USER; i++) {
    // if it is the user then it doesn't send the message to them
    if (user_list[i].m_user_id != sender && user_list[i].m_status == SLOT_FULL) {
      write(user_list[i].m_fd_write_to_child, buf, MAX_MSG);
    }
  }
  memset(buf, 0, sizeof(buf)); // clear buffer
  return 0;
}

/*
 * Cleanup user chat boxes
 */
void cleanup_users(USER *user_list) {
  // go over the user list and check for any empty slots
  // call cleanup user for each of those users.
  for (int i = 0; i < MAX_USER; i++) {
    if (user_list[i].m_status == SLOT_EMPTY) {
      cleanup_user(i, user_list);
    }
  }
}

/*
 * find user index for given user name
 */
int find_user_index(USER *user_list, char *user_id) {
  // go over the  user list to return the index of the user which matches the
  // argument user_id return -1 if not found
  int i, user_idx = -1;

  if (user_id == NULL) {
    fprintf(stderr, "NULL name passed.\n");
    return user_idx;
  }
  for (i = 0; i < MAX_USER; i++) {
    if (user_list[i].m_status == SLOT_EMPTY)
      continue;
    if (strcmp(user_list[i].m_user_id, user_id) == 0) {
      return i;
    }
  }

  return -1;
}

/*
 * given a command's input buffer, extract name
 */
int extract_name(char *buf, char *user_name) {
  char inbuf[MAX_MSG];
  char *tokens[16];
  strcpy(inbuf, buf);

  int token_cnt = parse_line(inbuf, tokens, " ");

  if (token_cnt >= 2) {
    strcpy(user_name, tokens[1]);
    return 0;
  }

  return -1;
}

int extract_text(char *buf, char *text) {
  char inbuf[MAX_MSG];
  char *tokens[16];
  char * s = NULL;
  strcpy(inbuf, buf);

  int token_cnt = parse_line(inbuf, tokens, " ");

  if(token_cnt >= 3) {
    //Find " "
    s = strchr(buf, ' ');
    s = strchr(s+1, ' ');
    strcpy(text, s+1);
    return 0;
  }

  return -1;
}

/*
 * send personal message
 */
void send_p2p_msg(int idx, USER *user_list, char *buf) {

  char target_message[MAX_MSG];
  char target_user[MAX_USER_ID];
  char temp_buf[MAX_MSG];
  int ret;

  extract_name(buf, target_user);
  ret = find_user_index(user_list, target_user);
  if (ret == -1) {
    write(user_list[idx].m_fd_write_to_child, "User not found", 15);
  } else {
    // Message format <user who sent message>: <message they sent>
    /****** BEGIN ********/
    extract_text(buf, target_message);
    strcpy(target_user, user_list[idx].m_user_id);
    strcat(target_user, ": ");
    strcpy(temp_buf, target_user);
    strcat(temp_buf, target_message);
    /****** END **********/
    write(user_list[ret].m_fd_write_to_child, temp_buf, MAX_MSG);
    memset(target_message, 0, MAX_MSG);
  }

  // get the target user by name using extract_name() function
  // find the user id using find_user_index()
  // if user not found, write back to the original user "User not found", using
  // the write()function on pipes. if the user is found then write the message
  // that the user wants to send to that user.
}

// takes in the filename of the file being executed, and prints an error message
// stating the commands and their usage

void show_error_message(char *filename) {
  /*
  sprintf("%s is not a valid command.\nValid commands are:\n"
  "\\list - List online users\n"
  "\\p2p <user_name> <message> - sends a private message to user\n"
  "\\exit - Disconnect from server"
  "<text> Will be broadcasted to users connected to the server\n", filename);
*/
}

/*
 * Populates the user list initially
 */
void init_user_list(USER *user_list) {

  // iterate over the MAX_USER
  // memset() all m_user_id to zero
  // set all fd to -1
  // set the status to be EMPTY
  int i = 0;
  for (i = 0; i < MAX_USER; i++) {
    user_list[i].m_pid = -1;
    memset(user_list[i].m_user_id, '\0', MAX_USER_ID);
    user_list[i].m_fd_write_to_server = -1;
    user_list[i].m_fd_read_from_child = -1;
    user_list[i].m_fd_write_to_child = -1;
    user_list[i].m_fd_read_from_server = -1;
    user_list[i].m_status = SLOT_EMPTY;
  }
}

/* ---------------------End of the functions that implementServer functionality-----------------*/

/* ---------------------Start of the Main function----------------------------------------------*/
int main(int argc, char *argv[]) {
  int nbytes;
  setup_connection("YOUR_UNIQUE_ID"); // Specifies the connection point as argument.

  USER user_list[MAX_USER];
  init_user_list(user_list); // Initialize user list

  char buf[MAX_MSG];
  char server_buf[MAX_MSG]; // for server buffer
  char child_buf[MAX_MSG]; // for child buffer
  char temp_buf[MAX_MSG]; // used for broadcast
  char *position; // pointer for eliminating newline
  int user_count = 0; // keeps track of how many users there are
  fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
  print_prompt("admin");

  while (1) {
    /* ------------------------YOUR CODE FOR MAIN--------------------------------*/

    // Handling a new connection using get_connection
    int pipe_SERVER_reading_from_child[2];
    int pipe_SERVER_writing_to_child[2];
    int pipe_child_writing_to_user[2];
    int pipe_child_reading_from_user[2];
    char user_id[MAX_USER_ID];

    // checks if connection was made if not, the else statement reads any input in buffer
    if (get_connection(user_id, pipe_child_writing_to_user, pipe_child_reading_from_user) != -1) {
      int empty_idx;
      int new_user_idx;
      int status;

      // this is to initialize the pipes for server
      pipe(pipe_SERVER_reading_from_child);
      pipe(pipe_SERVER_writing_to_child);
      empty_idx = find_empty_slot(user_list);


      // if slots are full it prevents server from forking and making more space
      // else forks and child-user connection is made
      if((find_empty_slot(user_list) == -1) || (find_user_index(user_list, user_id) != -1)) {
        printf("Slots full\n");
      } else {

        // making pipes non-blocking
        fcntl(pipe_SERVER_reading_from_child[0], F_SETFL, fcntl(pipe_SERVER_reading_from_child[0], F_GETFL) | O_NONBLOCK);
        fcntl(pipe_SERVER_reading_from_child[1], F_SETFL, fcntl(pipe_SERVER_reading_from_child[1], F_GETFL) | O_NONBLOCK);
        fcntl(pipe_SERVER_writing_to_child[1], F_SETFL, fcntl(pipe_SERVER_writing_to_child[1], F_GETFL) | O_NONBLOCK);
        fcntl(pipe_SERVER_writing_to_child[0], F_SETFL, fcntl(pipe_SERVER_writing_to_child[0], F_GETFL) | O_NONBLOCK);

        pid_t pidID = fork();

        // this is just to make it easier to understand which is which
        int fd_child_write_to_server = pipe_SERVER_reading_from_child[0];
        int fd_server_read_from_child = pipe_SERVER_reading_from_child[1];
        int fd_write_to_child = pipe_SERVER_writing_to_child[1];
        int fd_read_from_server = pipe_SERVER_writing_to_child[0];

        if (pidID == 0) { // child process w/2 additional pipes for bidirectional comms

          fcntl(pipe_child_reading_from_user[0], F_SETFL, fcntl(pipe_child_reading_from_user[0], F_GETFL) | O_NONBLOCK);
          fcntl(pipe_SERVER_reading_from_child[1], F_SETFL, fcntl(pipe_SERVER_reading_from_child[1], F_GETFL) | O_NONBLOCK);

          // continuous check if there is something in pipes to read or write
          while(1){
            // close used pipes in child process
            close(pipe_child_reading_from_user[1]);
            close(pipe_SERVER_reading_from_child[0]);
            close(pipe_SERVER_writing_to_child[1]);
            close(pipe_child_writing_to_user[0]);

            // while loop for input nonblock
            while (read(pipe_child_reading_from_user[0], buf, MAX_MSG) > 0) {
              usleep(600);
              write(pipe_SERVER_reading_from_child[1], buf, strlen(buf));
              memset(buf, 0, sizeof(buf)); // clear buffer
            }
            // while loop for reading server feedback
            while(read(pipe_SERVER_writing_to_child[0], child_buf, MAX_MSG) > 0) {
              usleep(600);
              write(pipe_child_writing_to_user[1], child_buf, MAX_MSG);
              memset(child_buf, 0, sizeof(child_buf));
            }
            usleep(600);
          }

          // exit status will be removed
          exit(status);
        } else {
          // parent process
          // adds users and their corresponding pipes
          add_user(empty_idx, user_list, pidID, user_id,fd_write_to_child,
            fd_child_write_to_server, fd_server_read_from_child, fd_read_from_server);
          user_count++;
          printf("\nUser connected: %s Slots in use: %d\n", user_id, user_count);
          print_prompt("admin");
        }
      }
    } else {


      /*this is to parse through every user and read from their input then do multiple
      else statements to check what commands the users send */

      for(int i = 0; i < MAX_USER; i++) {
        if (user_list[i].m_status == SLOT_FULL) {
          while (read(user_list[i].m_fd_write_to_server, buf, MAX_MSG) > 0) {

            if (start_with(user_commands[0], buf) == 0) {
              list_users(find_user_index(user_list, user_list[i].m_user_id), user_list);
              printf("User request to see online partcipants");

              /* insert more admin commands here */
               // clear buffer
              memset(buf, 0, sizeof(buf));
            } else if (start_with(user_commands[2], buf) == 0){
                // private messaging from user to user
                send_p2p_msg(i, user_list, buf);
                printf("Private message was sent");
                // clear buffer
                memset(buf, 0, sizeof(buf));
            } else if (start_with(user_commands[4], buf) == 0){
                // kicks all users in server
                kick_user(i, user_list);

                //clear buffer
                memset(buf, 0, sizeof(buf));
            } else {
              // this adds the admin: <message> attachment to the broadcasted message
              strcpy(temp_buf, user_list[i].m_user_id);
              strcat(temp_buf, ": ");
              strcat(temp_buf, buf);
              broadcast_msg(user_list, temp_buf, user_list[i].m_user_id);
              printf("User broadcasted message: %s", buf);
              // clear buffers
              memset(buf, 0, sizeof(buf));
              memset(temp_buf, 0, sizeof(temp_buf));
            }
            printf("\n");
            print_prompt("admin");
          }
          usleep(50);
        }
      }

      // nonblocking read from stdin for admin side
      while(read(0, server_buf, MAX_MSG) > 0) {

        if (strlen(server_buf) > 0) {
          if ((position=strchr(server_buf,'\n')) != NULL) {
            *position = '\0';
          } else {
            perror("Error at reading:");
          }
          // list users
          if (start_with(user_commands[0], server_buf) == 0) {
            list_users(-1, user_list);
            // clear buffer
            memset(server_buf, 0, sizeof(server_buf));

            //kick user
          } else if (start_with(user_commands[1], server_buf) == 0) {
            extract_name(server_buf, temp_buf);
            kick_user(find_user_index(user_list, temp_buf), user_list);
            user_count--;
            //clear buffers
            memset(server_buf, 0, sizeof(server_buf));
            memset(temp_buf, 0, sizeof(temp_buf));

            //exit from admin side
          }  else if (start_with(user_commands[4], server_buf) == 0) {
            for(int idx = 0; idx < MAX_USER; idx++) {
              // if it is the user then it doesn't send the message to them
              if (user_list[idx].m_status == SLOT_FULL) {
                kick_user(idx, user_list);
              }
            }
            cleanup_users(user_list);
            user_count = 0;
            //clear buffers
            memset(server_buf, 0, sizeof(server_buf));

            //broadcast to users
          } else {
            // this adds the admin: <message> attachment to the broadcasted message
            strcpy(temp_buf, "Notice: ");
            strcat(temp_buf, server_buf);
            broadcast_msg(user_list, temp_buf, "admin");
            // clear buffers
            memset(server_buf, 0, sizeof(server_buf));
            memset(temp_buf, 0, sizeof(temp_buf));
          }
          print_prompt("admin");
        }
      }
    }
    usleep(50);
    // Check max user and same user id

    // Child process: poli users and SERVER

    // Server process: Add a new user information into an empty slot
    // poll child processes and handle user commands
    // Poll stdin (input from the terminal) and handle admnistrative command

    /* ------------------------YOUR CODE FOR MAIN--------------------------------*/
  }
}

/* --------------------End of the main function----------------------------------------*/

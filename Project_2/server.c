#include "comm.h"
#include "util.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

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
    printf(buf);
    printf("\n");
  } else {
    /* write to the given pipe fd */
    if (write(user_list[idx].m_fd_to_user, buf, strlen(buf) + 1) < 0)
      perror("writing to server shell");
  }

  return 0;
}

/*
 * add a new user
 */
int add_user(int idx, USER *user_list, int pid, char *user_id,
             int pipe_to_child, int pipe_to_parent) {
  // populate the user_list structure with the arguments passed to this function
  strcpy(user_list[idx].m_user_id, user_id);
  user_list[idx].m_pid = pid;
  user_list[idx].m_fd_to_user= pipe_to_child;
  user_list[idx].m_fd_to_server = pipe_to_parent;
  user_list[idx].m_status = SLOT_FULL;
  // return the index of user added
  return idx;
}

/*
 * Kill a user
 */
void kill_user(int idx, USER *user_list) {
  // kill a user (specified by idx) by using the systemcall kill()
  // then call waitpid on the user
}

/*
 * Perform cleanup actions after the used has been killed
 */
void cleanup_user(int idx, USER *user_list) {
  // m_pid should be set back to -1
  // m_user_id should be set to zero, using memset()
  // close all the fd
  // set the value of all fd back to -1
  // set the status back to empty
}

/*
 * Kills the user and performs cleanup
 */
void kick_user(int idx, USER *user_list) {
  // should kill_user()
  // then perform cleanup_user()
}

/*
 * broadcast message to all users
 */
int broadcast_msg(USER *user_list, char *buf, char *sender) {
  // iterate over the user_list and if a slot is full, and the user is not the
  // sender itself, then send the message to that user return zero on success
  return 0;
}

/*
 * Cleanup user chat boxes
 */
void cleanup_users(USER *user_list) {
  // go over the user list and check for any empty slots
  // call cleanup user for each of those users.
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

  // get the target user by name using extract_name() function
  // find the user id using find_user_index()
  // if user not found, write back to the original user "User not found", using
  // the write()function on pipes. if the user is found then write the message
  // that the user wants to send to that user.
}

// takes in the filename of the file being executed, and prints an error message
// stating the commands and their usage
void show_error_message(char *filename) {}

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
    user_list[i].m_fd_to_user = -1;
    user_list[i].m_fd_to_server = -1;
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
    char server_Buff[30];
    char child_Buff[30];
    int empty_idx;
    int new_user_idx;

    // which pipes read from user and what do the pipes in get connection act as

    // if get_connect == True
    // check user list
    // then fork
    if (get_connection(user_id, pipe_child_writing_to_user, pipe_child_reading_from_user) != -1) {
      printf("\nConnection made\nUserID: %s\n", user_id);
      //memset
      // this is to initialize the pipes for server
      if (pipe(pipe_SERVER_reading_from_child) == -1) {
      	fprintf(stderr, "Pipe Failed");
      	return 1;
      }
      if (pipe(pipe_SERVER_writing_to_child) == -1) {
        fprintf(stderr, "Pipe Failed");
        return 1;
      }
      pid_t pidID = fork();
      if (pidID == 0) { // child process w/2 additional pipes for bidirectional comms
        // while loop for input
        // nonblock
        while (read(pipe_child_reading_from_user[0], child_Buff, 30) == -1) {
          usleep(600000);
        }
        // close(pipe_child_reading_from_user[0]);
        write(pipe_SERVER_reading_from_child[1], child_Buff, strlen(child_Buff));
        memset(child_Buff, 0, sizeof(child_Buff)); // clear buffer

      } else {  // parent process
        printf("Function Empty: %d\n", find_empty_slot(user_list));
        empty_idx = find_empty_slot(user_list);
        if (empty_idx == -1) { // checks if theres an empty slot for new user
          printf("All slots full\n");
        } else {
          printf("Empty idx %d\n", empty_idx);
          new_user_idx = add_user(empty_idx, user_list, pidID, user_id, pipe_SERVER_writing_to_child[1], pipe_SERVER_reading_from_child[0]); // adds new user to slot
          printf("User Index %d\n", new_user_idx);
        }
      }
      // close(pipe_SERVER_reading_from_child[0]);
    } else {
      /*this is to parse through every user and read from their input then do multiple
      else statements to check what commands the users send */

      for(int i = 0; i < MAX_USER; i++) {
        if (user_list[i].m_status == SLOT_FULL) {
          while (read(user_list[i].m_fd_to_server, server_Buff, 30) == -1) {;}
          printf("Server output: %s\n", server_Buff);
          printf("UserPid: %d\n", user_list[i].m_pid);
          printf("UserSlot: %d\n", user_list[i].m_status);
          printf("UserID: %s\n", user_list[i].m_user_id);
          memset(server_Buff, 0, sizeof(server_Buff)); // clear buffer
        }
      }
    }
    //printf("test\n");
    // Check max user and same user id

    // Child process: poli users and SERVER

    // Server process: Add a new user information into an empty slot
    // poll child processes and handle user commands
    // Poll stdin (input from the terminal) and handle admnistrative command

    /* ------------------------YOUR CODE FOR MAIN--------------------------------*/
  }
}

/* --------------------End of the main function----------------------------------------*/

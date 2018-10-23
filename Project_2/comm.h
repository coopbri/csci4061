#ifndef __COMM_H__
#define __COMM_H__

#include <unistd.h>

#define MAX_MSG 256
#define MAX_USER_ID 32
#define MAX_USER 10

typedef enum slot_status {
	SLOT_FULL =0,
	SLOT_EMPTY = 1
} SLOT_STATUS;

typedef struct _userInfo {
	int m_pid;
	char m_user_id[MAX_USER_ID];
	int m_fd_to_user;
	int m_fd_to_server;
	int m_status;
} USER;

int recv_fd(int socket, int n, int* fds);
void send_fd(int socket, int *fds, int n);
int connect_to_server(char * connect_point, char * user_id, int pipe_to_user[2], int pipe_to_write[2]);
int setup_connection(char * connect_point);
int get_connection(char * user_id, int pipe_to_user[2], int pipe_to_server[2]);

/*
 * user slot status
 */

/*
 * Structure for the server
 */
//typedef struct server_controller_s {
//	int ptoc[2];
//	int ctop[2];
//	pid_t pid;
//	pid_t child_pid;
//} server_ctrl_t;

/*
 * Structure for user chat boxes
 */
//pedef struct user_chat_box_s {
//nt ptoc[2];
//nt ctop[2];
//har name[MAX_MSG];
//id_t pid;
//id_t child_pid;
//nt status;
//user_chat_box_t;

//int starts_with(const char *a, const char *b);

#endif

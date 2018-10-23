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
int connect_to_server(char * connect_point, char * user_id, int pipe_user_reading_from_server[2], int pipe_user_writing_to_server[2]);
int setup_connection(char * connect_point);
int get_connection(char * user_id, int pipe_child_writing_to_user[2], int pipe_child_reading_from_user[2]);

#endif

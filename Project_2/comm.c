#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/un.h>
#include "comm.h"

int g_sfd;

void send_fd(int socket, int *fds, int n)  // send fd by socket
{
	struct msghdr msg = {0};
	struct cmsghdr *cmsg;
	char buf[CMSG_SPACE(n * sizeof(int))], dup[256];
	memset(buf, '\0', sizeof(buf));
	struct iovec io = { .iov_base = &dup, .iov_len = sizeof(dup) };

	msg.msg_iov = &io;
	msg.msg_iovlen = 1;
	msg.msg_control = buf;
	msg.msg_controllen = sizeof(buf);

	cmsg = CMSG_FIRSTHDR(&msg);
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	cmsg->cmsg_len = CMSG_LEN(n * sizeof(int));

	memcpy ((int *) CMSG_DATA(cmsg), fds, n * sizeof (int));

	if (sendmsg (socket, &msg, 0) < 0) {
		printf("Failed to send message");
	}	
}

int recv_fd(int socket, int n, int* fds) {
	struct msghdr msg = {0};
	struct cmsghdr *cmsg;
	char buf[CMSG_SPACE(n * sizeof(int))], dup[256];
	memset(buf, '\0', sizeof(buf));
	struct iovec io = { .iov_base = &dup, .iov_len = sizeof(dup) };

	msg.msg_iov = &io;
	msg.msg_iovlen = 1;
	msg.msg_control = buf;
	msg.msg_controllen = sizeof(buf);

	if (recvmsg (socket, &msg, 0) < 0) {
		printf("Failed to receive message");
		return -1;
	}

	cmsg = CMSG_FIRSTHDR(&msg);
	memcpy (fds, (int *) CMSG_DATA(cmsg), n * sizeof(int));
	return 0;
}

int connect_to_server(char * connect_point, char * user_id, int pipe_user_reading_from_server[2], int pipe_user_writing_to_server[2])
{
	signal(SIGPIPE,SIG_IGN);
	struct sockaddr_un addr;
	int server_fd;

	server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (server_fd == -1) {
		printf("Failed to connect server");
		return -1;
	}

	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;

	char socket_address[256];
	sprintf(socket_address, "/tmp/%s.socket", connect_point);
	strncpy(addr.sun_path, socket_address, sizeof(addr.sun_path) -1);

	if (connect(server_fd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1) {
		printf("Failed to connect to socket\n");
		return -1;
	}

	if (write(server_fd, user_id, MAX_USER_ID) == -1) {
		perror("Failed to write user id\n");
		return -1;
	}

	if (recv_fd(server_fd, 2, pipe_user_reading_from_server) !=0) {
		printf("Error in recv_fd\n");
		return -1;
	}

	if (recv_fd(server_fd, 2, pipe_user_writing_to_server) != 0) {
		printf("Error in recv_fd\n");
		return -1;
	}

	close(server_fd);

	return 0;
}

int setup_connection(char * connect_point) 
{
	signal(SIGPIPE,SIG_IGN);
	struct sockaddr_un addr;

	g_sfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (g_sfd == -1) {
		perror("Failed to create socket");
		return -1;
	}


	char socket_address[256];
	sprintf(socket_address, "/tmp/%s.socket", connect_point);

	if (unlink(socket_address) == -1 && errno != ENOENT) {
		perror("Removing socket file failed");
		return -1;
	}

	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, socket_address, sizeof(addr.sun_path) -1);

	if (bind(g_sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1) {
		perror("Failed to bind to socket");
		return -1;
	}

	if (listen(g_sfd, 5) == -1) {
		perror("Failed to listen on socket");
		return -1;
	}

	printf("Wating user's connection.\n");
	fcntl(g_sfd, F_SETFL, O_NONBLOCK);
}

int get_connection(char * user_id, int pipe_child_writing_to_user[2], int pipe_child_reading_from_user[2])
{
	int cfd = accept(g_sfd, NULL, NULL);

	if (cfd != -1) {
		//fork and sotre client info
		//int ret = fork();

		if (pipe(pipe_child_writing_to_user) < 0 || pipe(pipe_child_reading_from_user) < 0) {
			perror("Failed to create pipe\n");
			return -1;
		}

		send_fd(cfd, pipe_child_writing_to_user, 2);
		send_fd(cfd, pipe_child_reading_from_user, 2);

		if(read(cfd, user_id, MAX_USER_ID) == -1) {
			perror("Failed to get user id");
			return -1;
		}
		close(cfd);
	}
	else
		return -1;

	return 0;

}

#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>


static int master_fd = -1;
pthread_mutex_t accept_con_mutex = PTHREAD_MUTEX_INITIALIZER;

int makeargv(const char *s, const char *delimiters, char ***argvp) {
   int error;
   int i;
   int numtokens;
   const char *snew;
   char *t;

   if ((s == NULL) || (delimiters == NULL) || (argvp == NULL)) {
      errno = EINVAL;
      return -1;
   }
   *argvp = NULL;
   snew = s + strspn(s, delimiters);
   if ((t = malloc(strlen(snew) + 1)) == NULL)
      return -1;
   strcpy(t,snew);
   numtokens = 0;
   if (strtok(t, delimiters) != NULL)
      for (numtokens = 1; strtok(NULL, delimiters) != NULL; numtokens++) ;

   if ((*argvp = malloc((numtokens + 1)*sizeof(char *))) == NULL) {
      error = errno;
      free(t);
      errno = error;
      return -1;
   }

   if (numtokens == 0)
      free(t);
   else {
      strcpy(t,snew);
      **argvp = strtok(t,delimiters);
      for (i=1; i<numtokens; i++)
         *((*argvp) +i) = strtok(NULL,delimiters);
   }
   *((*argvp) + numtokens) = NULL;
   return numtokens;
}

void freemakeargv(char **argv) {
   if (argv == NULL)
      return;
   if (*argv != NULL)
      free(*argv);
   free(argv);
}

/**********************************************
 * init
   - port is the number of the port you want the server to be
     started on
   - initializes the connection acception/handling system
   - YOU MUST CALL THIS EXACTLY ONCE (not once per thread,
     but exactly one time, in the main thread of your program)
     BEFORE USING ANY OF THE FUNCTIONS BELOW
   - if init encounters any errors, it will call exit().
************************************************/
enum boolean {FALSE, TRUE};
void init(int port) {
   // set up sockets using bind, listen
   // also do setsockopt(SO_REUSEADDR)
   // exit if the port number is already in use

   int sd;
   struct sockaddr_in addr;
   int ret_val;
   enum boolean flag;

   sd = socket(AF_INET, SOCK_STREAM, 0);
   if(sd < 0) {
      perror("init(): socket() error!");
      exit(1);
   }
   flag = TRUE;
   ret_val = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char*) &flag, sizeof(flag));

   if (ret_val == -1) {
      perror("init(): setsockopt(SO_REUSEADDR) error!");
      exit(1);
   }

   addr.sin_family = AF_INET;
   addr.sin_addr.s_addr = INADDR_ANY;
   addr.sin_port = htons(port);

   if((bind(sd, (struct sockaddr*) &addr, sizeof(addr))) == -1) {
      perror("init(): bind() error!");
      exit(1);
   }

   if ((listen(sd, 20)) == -1) {
      perror("init(): listen() error!");
      exit(1);
   }

   master_fd = sd;

}

/**********************************************
 * accept_connection - takes no parameters
   - returns a file descriptor for further request processing.
     DO NOT use the file descriptor on your own -- use
     get_request() instead.
   - if the return value is negative, the thread calling
     accept_connection must should ignore request.
***********************************************/
int accept_connection(void) {

   // this needs locking around it completely.
   // accept one connection using accept()
   // return the fd returned by accept()
   if (pthread_mutex_lock(&accept_con_mutex) < 0) {
   		printf("Failed to lock connection mutex.");
   }

   int newsock;
   struct sockaddr_in new_recv_addr;
   int addr_len;

   addr_len = sizeof(new_recv_addr);

   newsock = accept(master_fd, (struct sockaddr *)&new_recv_addr, &addr_len);

   if (pthread_mutex_unlock(&accept_con_mutex) < 0) {
   		printf("Failed to unlock connection mutex.");
   }

   return newsock;

}

/**********************************************
 * get_request
   - parameters:
      - fd is the file descriptor obtained by accept_connection()
        from where you wish to get a request
      - filename is the location of a character buffer in which
        this function should store the requested filename. (Buffer
        should be of size 1024 bytes.)
   - returns 0 on success, nonzero on failure. You must account
     for failures because some connections might send faulty
     requests. This is a recoverable error - you must not exit
     inside the thread that called get_request. After an error, you
     must NOT use a return_request or return_error function for that
     specific 'connection'.
************************************************/
int get_request(int fd, char *filename) {
   // this does not need locking. (as long as one thread per fd)
   // read from the socketfd and parse the first line for the GET info
   // if it isn't a GET request, then just dump it and return -1.
   // otherwise, store the path/filename into 'filename'.

   char buf[2048];
   FILE *stream = fdopen(fd,"r");
	if (stream == NULL) {
		printf("Failed to open connection to client: %s", strerror(errno));
		return -4;
	}

   if (fgets(buf,2048,stream) == NULL) {
   	  printf("Fgets failed to read from client: %s", strerror(errno));
      close(fd);
      return -3;
   }

   char **strings;
   int args = makeargv(buf," \n",&strings);
   if (args < 2) {
      close(fd);
      return -2;
   }

   if (strcmp("GET",strings[0])!=0) {
   	  printf("Not a GET command");
      close(fd);
      return -1;
   }

   // Makeargv had an issue.
   if (strings[1] == NULL) {
     printf("Error in parsing request. \n");
   		close(fd);
    	return -5;
    }

   if (strstr(strings[1],"..") != NULL || strstr(strings[1],"//") != NULL) {
   	  printf("Attempted security breach.");
      close(fd);
      return -4;
   }

   if (strlen(strings[1]) > 1024) {
   		printf("BUFFER OVERFLOW IMMINENT!");
   }

   strncpy(filename,strings[1],1024);
   filename[1023] = '\0'; // just in case

   return 0;

   // [[read from the socket until we get a blank line, then that's our
   //    'signal' to stop reading from it.]] -- necessary? lets try no.
}

/**********************************************
 * return_result
   - returns the contents of a file to the requesting client
   - parameters:
      - fd is the file descriptor obtained by accept_connection()
        to where you wish to return the result of a request
      - content_type is a pointer to a string that indicates the
        type of content being returned. possible types include
        "text/html", "text/plain", "image/gif", "image/jpeg" cor-
        responding to .html, .txt, .gif, .jpg files.
      - buf is a pointer to a memory location where the requested
        file has been read into memory (the heap). return_result
        will use this memory location to return the result to the
        user. (remember to use -D_REENTRANT for CFLAGS.) you may
        safely deallocate the memory after the call to
        return_result (if it will not be cached).
      - numbytes is the number of bytes the file takes up in buf
   - returns 0 on success, nonzero on failure.
************************************************/
int return_result(int fd, char *content_type, char *buf, int numbytes) {
   // send headers back to the socketfd, connection: close, content-type, content-length, etc
   // then finally send back the resulting file
   // then close the connection

   FILE *stream = fdopen(fd,"w");

if (stream == NULL) {
	printf("Failed to open stream.\n");
	return -1;
} else {

   if (fprintf(stream,"HTTP/1.0 200 OK\nContent-Type: %s\nContent-Length: %d\nConnection: Close\n\n", content_type, numbytes) < 0) {

   		printf("Failed to print to stream.\n");
   }
   if (fflush(stream) < 0) {

   		printf("Failed to flush stream.\n");
   }

   if (write(fd,buf,numbytes) < 0) {
   		printf("Failed to write data back to client: %s", strerror(errno));
   }

   close(fd);
}
}


/**********************************************
 * return_error
   - returns an error message in response to a bad request
   - parameters:
      - fd is the file descriptor obtained by accept_connection()
        to where you wish to return the error
      - buf is a pointer to the location of the error text
   - returns 0 on success, nonzero on failure.
************************************************/
int return_error(int fd, char *buf) {
   // send 404 headers back to the socketfd, connection: close, content-type: text/plain, content-length
   // send back the error message as a piece of text.
   // then close the connection

   FILE *stream = fdopen(fd,"w");

   fprintf(stream,"HTTP/1.0 404 Not Found\nContent-Length: %d\nConnection: Close\n\n",strlen(buf));
   fflush(stream);

   write(fd,buf,strlen(buf));
   close(fd);

}

// includes:
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include "util.h"
#include <stdbool.h>
#include <unistd.h>

void *dispatch(void *arg);
void *worker(void * arg);

// defines:
#define MAX_THREADS 100
#define MAX_QUEUE_LEN 100
#define MAX_CE 100
#define INVALID -1
#define BUFF_SIZE 1024

pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t dynamic_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_cv = PTHREAD_COND_INITIALIZER;
int queue_fill_slot = 0;
int queue_grab_slot = 0;
int cache_size = 0;
int queue_length = 0;
int cache_slot = 0;
int dynamic_flag = 0;
int num_dispatchers = 0;
int num_workers = 0;
pthread_t *dispatchers;
pthread_t *workers;
char *path;

/*
  THE CODE STRUCTURE GIVEN BELOW IS JUST A SUGGESTION. FEEL FREE TO MODIFY AS NEEDED
*/

// structs:
typedef struct request_queue {
   int fd;
   void *request;
} request_t;

typedef struct cache_entry {
    int len;
    char *request;
    char *content;
} cache_entry_t;

request_t *queue_buffer;
cache_entry_t **cache_buffer;

/* ************************ Dynamic Pool Code ***********************************/
// Extra Credit: This function implements the policy to change the worker thread pool dynamically
// depending on the number of requests
// TODO: If we have time for the extra credit
void* dynamic_pool_size_update(void *arg) {
  printf("Not implemented\n");
  return -1;
}
/**********************************************************************************/

/* ************************************ Cache Code ********************************/

// Function to check whether the given request is present in cache
// TODO: Jared
int getCacheIndex(char *request){
  /// return the index if the request is present in the cache
  for (int i = 0; i < cache_size; i++) {
    if(cache_buffer[i]->request == NULL) {
      return -1;
    }
    if(strcmp(cache_buffer[i]->request, request) == 0){
      return i;
    }
  }
}

// Function to add the request and its file content into the cache
// TODO: Jared
void addIntoCache(char *request, char *file , int memory_size){
  // It should add the request at an index according to the cache replacement policy
  // Make sure to allocate/free memory when adding or replacing cache entries
  /*
    if = cache at max size print "its full bud"
    else = goes into a check if pointer is present
  */
  if (cache_buffer[cache_slot]->len){
    /*
      if pointer is present we free up that slot
      then we place our new entry into that slot
      increment the size of cache
    */
    free(cache_buffer[cache_slot]->request);
    free(cache_buffer[cache_slot]->content);
    cache_buffer[cache_slot]->request = malloc(BUFF_SIZE * sizeof(char));
    cache_buffer[cache_slot]->content = malloc(memory_size * sizeof(char));
    strcpy(cache_buffer[cache_slot]->request, request);
    memcpy(cache_buffer[cache_slot]->content, file, memory_size);
    cache_buffer[cache_slot]->len = memory_size;
    cache_slot = (cache_slot + 1) % cache_size;
  } else {
    /*
      if no pointer is found then we just place the
      pointer to struct into the slot
    */
    cache_buffer[cache_slot]->request = malloc(BUFF_SIZE * sizeof(char));
    cache_buffer[cache_slot]->content = malloc(memory_size * sizeof(char));
    strcpy(cache_buffer[cache_slot]->request, request);
    memcpy(cache_buffer[cache_slot]->content, file, memory_size);
    cache_buffer[cache_slot]->len = memory_size;
    cache_slot = (cache_slot + 1) % cache_size;
  }
}

// clear the memory allocated to the cache
// TODO: Jared
void deleteCache(){
  // De-allocate/free the cache memory
  // frees the pointers within the pointer array first
  for (int i=0; i<cache_size; i++) {
    free(cache_buffer[i]->request);
    free(cache_buffer[i]->content);
  }
  // then we free the pointer array
  free(cache_buffer);
}

// Function to initialize the cache
// TODO: Jared
void initCache(){
  // Allocating memory and initializing the cache array
  // creates an array of pointers, which these pointers point to structs
  cache_buffer = (struct cache_entry **)malloc(sizeof(struct cache_entry *) * cache_size);
  for(int i = 0; i < cache_size; i++) {
    cache_buffer[i] = malloc(sizeof(struct cache_entry));
  }
}

// Function to open and read the file from the disk into the memory
// Add necessary arguments as needed
// TODO: Brian
int readFromDisk(char *request, struct stat *stat_buff, char **buf) {
  FILE *f;
  char *abs_path = malloc(sizeof(char) * (strlen(request) + strlen(path)) + 1);
  // Open and read the contents of file given the request
  strcat(abs_path, path);
  strcat(abs_path, request);
  f = fopen(abs_path, "r");
  if (f == NULL || stat(abs_path, stat_buff) != 0) {
    free(abs_path);
    return -1;
  }
  (*buf) = (char *)malloc(stat_buff->st_size * sizeof(char) + 1);
  fread(*buf, stat_buff->st_size, 1, f);
  fclose(f);
  free(abs_path);
  return 0;
}

/**********************************************************************************/

/* ************************************ Utilities ********************************/
// Function to get the content type from the request
// TODO: Brian
char* getContentType(char *mybuf) {
  // Should return the content type based on the file type in the request
  // (See Section 5 in Project description for more details)

  int path_len = strlen(mybuf);
  char *content_type = malloc(13*sizeof(char));

  if (path_len > 5 && strcmp(mybuf + path_len - 5, ".html") == 0) {
    // file type is 'text/html'
    strcpy(content_type, "text/html\n");
  } else if (path_len > 4 && strcmp(mybuf + path_len - 4, ".jpg") == 0) {
    // file type is 'image/jpeg'
    strcpy(content_type, "image/jpeg\n");
  } else if (path_len > 4 && strcmp(mybuf + path_len - 4, ".gif") == 0) {
    // file type is 'image/gif'
    strcpy(content_type, "image/gif");
  } else {
    // file type is 'text/plain'
    strcpy(content_type, "text/plain\n");
  }
  return content_type;
}

// This function returns the current time in microseconds
long getCurrentTimeInMillis() {
  struct timeval curr_time;
  gettimeofday(&curr_time, NULL);
  return curr_time.tv_sec * 1000000 + curr_time.tv_usec;
}

/**********************************************************************************/

// Function to receive the request from the client and add to the queue
// TODO: Chase
void * dispatch(void *arg) {
  char filebuf[1024];
  request_t request;
  int fd = 0;
  if(dynamic_flag == 1) {
    fd = *(int *) arg;
  }
  while (1) {
    // Accept client connection

    while(fd < 3 || dynamic_flag == 1) {
      fd = accept_connection();
    }

    // Get request from the client
    if(get_request(fd, filebuf) != 0) {
      printf("Failed!\n");
    }

    // Add the request into the queue
    if(pthread_mutex_lock(&queue_lock) < 0) {
      printf("Failed to lock queue mutex\n");
    }

    while(queue_fill_slot == queue_length) {
      pthread_cond_wait(&queue_cv, &queue_lock);
    }

    request.fd = fd;
    request.request = filebuf;
    queue_buffer[queue_fill_slot] = request;
    queue_fill_slot = (queue_fill_slot + 1) % queue_length;

    fd = 0;
    usleep(100);
    if(pthread_mutex_unlock(&queue_lock) < 0) {
      printf("Failed to unlock queue mutex\n");
    }
    pthread_cond_signal(&queue_cv);
  }
  return NULL;
}

/**********************************************************************************/

// Function to retrieve the request from the queue, process it and then return a result to the client
// TODO: Chase
void * worker(void *arg) {
  int pthread_num = *(int *) arg;
  int requests_handled = 0;
  int ms_time = 0;
  int fd = 0;
  int log_fd = 0;
  int index = 0;
  char *request;
  char log[BUFF_SIZE];

  while (1) {
    if(pthread_mutex_lock(&queue_lock) < 0) {
      printf("Failed to lock queue mutex\n");
    }

    while(queue_fill_slot == queue_grab_slot) {
      pthread_cond_wait(&queue_cv, &queue_lock);
    }
    // Start recording time
    ms_time = getCurrentTimeInMillis();

    // Get the request from the queue
    fd = queue_buffer[queue_grab_slot].fd;
    request = queue_buffer[queue_grab_slot].request;
    queue_grab_slot = (queue_grab_slot + 1) % queue_length;

    // Get the data from the disk or the cache
    index = getCacheIndex(request);
    if(index < 0) {
      struct stat stat_buff;
      char *file;
      if(readFromDisk(request, &stat_buff, &file) < 0) {
        ms_time = (getCurrentTimeInMillis() - ms_time);
        requests_handled++;
        sprintf(log, "[%d][%d][%d][%s][%d][%d ms][MISS]\n", pthread_num, requests_handled, fd, request, errno, ms_time);
        printf("%s", log);
        return_error(fd, strerror(errno));
        errno = 0;
        free(file);
      } else {
        addIntoCache(request, file, stat_buff.st_size);
        // Stop recording the time
        ms_time = (getCurrentTimeInMillis() - ms_time);
        requests_handled++;
        sprintf(log, "[%d][%d][%d][%s][%ld][%d ms][MISS]\n", pthread_num, requests_handled, fd, request, stat_buff.st_size, ms_time);
        printf("%s", log);
        // Log the request into the file and terminal
        // return the result
        return_result(fd, getContentType(request), file, stat_buff.st_size);
        free(file);
        usleep(100);
      }
    } else {
      // Stop recording the time
      ms_time = (getCurrentTimeInMillis() - ms_time);
      requests_handled++;
      sprintf(log, "[%d][%d][%d][%s][%d][%d ms][HIT]\n", pthread_num, requests_handled, fd, request, cache_buffer[index]->len, ms_time);
      printf("%s", log);
      // Log the request into the file and terminal
      // return the result
      return_result(fd, getContentType(request), cache_buffer[index]->content, cache_buffer[index]->len);
      usleep(100);
    }
    log_fd = open("web_server_log", O_APPEND | O_CREAT | O_WRONLY, 0777);
    if(log_fd < 0) {
      perror("\n");
      printf("Error setting up log\n");
    }
    if(write(log_fd, log, strlen(log)) < strlen(log)) {
      printf("Something went wrong when writing to log\n");
    }
    close(log_fd);
    ms_time = 0;
    if(pthread_mutex_unlock(&queue_lock) < 0) {
      printf("Failed to lock queue mutex\n");
    }
    pthread_cond_signal(&queue_cv);
  }
  return NULL;
}

/**********************************************************************************/
// TODO: Finish error checking and such. Chase & Brian
int main(int argc, char **argv) {
  int port;

  // -----------------------------ARGUMENT SETUP AND ERROR CHECKING----------------------------- //
  // Error check on number of arguments
  // Decide to check if caching is enabled [argc == 8 -> Caching enabled]
  if(argc != 7 && argc != 8) {
    printf("usage: %s port path num_dispatchers num_workers dynamic_flag queue_length cache_size\n", argv[0]);
    return -1;
  }

  port = atoi(argv[1]);
  // Verify port is in the proper range.
  if (port < 1025 || port > 65535) {
    printf("Port out of range, must be between 1025 and 65535.\n");
    return -1;
  }

  path = argv[2];
  if(strlen(path) > BUFF_SIZE) {
    printf("Path cannot exceed a length greater than %d.\n", BUFF_SIZE);
    return -1;
  }

  num_dispatchers = atoi(argv[3]);
  // Verify proper number of dispatch threads.
  if (num_dispatchers <= 0 || num_dispatchers > MAX_THREADS) {
    printf("Invalid number of dispatcher threads. Please enter a number between 0 and %d.\n", MAX_THREADS);
    return -1;
  }

  num_workers = atoi(argv[4]);
  // Verify proper number of worker threads.
  if (num_workers <= 0 || num_workers > MAX_THREADS) {
    printf("Invalid number of worker threads. Please enter a number between 0 and %d.\n", MAX_THREADS);
    return -1;
  }

  // Part of extra credit.
  dynamic_flag = atoi(argv[5]);
  // Check to make sure the dynamic flag was set properly.
  if (dynamic_flag > 1 || dynamic_flag < 0) {
    printf("Dynamic flag must be 0 (off) or 1 (on).\n");
    return -1;
  }

  queue_length = atoi(argv[6]);
  // Make sure the given queue size is <= maximum
  if (queue_length > MAX_QUEUE_LEN) {
    printf("Queue length exceeds maximum (%d).\n", MAX_QUEUE_LEN);
    return -1;
  }

  // if caching is enabled, generate cache
  if (argc == 8) {
    cache_size = atoi(argv[7]);
    // Make sure the given cache size is <= maximum
    if (cache_size > MAX_CE) {
      printf("Cache size exceeds maximum (%d).\n", MAX_CE);
      return -1;
    }
  } else {
    cache_size = 0;
  }
  // --------------------------END OF ARGUMENT SETUP AND ERROR CHECKING-------------------------- //


  // Initialize server on port from command line
  init(port);

  // Change the current working directory to server root directory
  chdir(path);

  // Start the server and initialize cache
  printf("Starting server on port %d: %d dispatchers, %d workers\n", port, num_dispatchers, num_workers);
  initCache();
  // Create the queue array (bounded buffer)
  queue_buffer = malloc(queue_length * sizeof(queue_buffer));
  // Initialize arrays for both thread types
  workers = malloc(num_workers * sizeof(pthread_t));
  dispatchers = malloc(num_dispatchers * sizeof(pthread_t));

  if(dynamic_flag == 0) {
    for (int i = 0; i < num_dispatchers; i++) {
      if (pthread_create(&(dispatchers[i]), NULL, dispatch, NULL) != 0) {
        printf("Error creating dispatcher thread.\n");
        exit(-1);
      }
    }

    for (int i = 0; i < num_workers; i++) {
      if (pthread_create(&(workers[i]), NULL, worker, (void *)&i) != 0) {
        printf("Error creating worker thread.\n");
        exit(-1);
      }
    }

    for (int i = 0; i < num_dispatchers; i++) {
      if (pthread_join(dispatchers[i], NULL) != 0) {
        printf("Error joining dispatcher thread.\n");
      }
    }
  } else {
    pthread_t dynamic_thread;
    if(pthread_create(&dynamic_thread, NULL, dynamic_pool_size_update, NULL) != 0) {
      printf("Error creating dispatcher thread.\n");
    }
    if(pthread_join(dynamic_thread, NULL) != 0) {
      printf("Error detaching thread.\n");
    }
  }

  // Clean up
  free(queue_buffer);
  deleteCache();
  // probably need to free threads, queue, cache
  // and destroy locks
  return 0;
}

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

static int bufin = 0;
static int bufout = 0;
static int total_requests = 0;
static int cache_size = 0;
static int queue_length = 0;

pthread_mutex_t error_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t buffer_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t read_lock = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t requests = PTHREAD_COND_INITIALIZER;
pthread_cond_t slots = PTHREAD_COND_INITIALIZER;

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

static request_t *queue_buffer;
static cache_entry_t *cache_buffer;

// Used to set the error if there is one for creating threads.
int seterror(int error) {
  int terror;
  if(!error) {
    return error;
  }
  if(terror = pthread_mutex_lock(&error_lock)) {
    return terror;
  }
  terror = pthread_mutex_unlock(&error_lock);
  return terror? terror: error;
}

// Used to initialize a single worker and set and error if the create fails.
int init_worker(pthread_t *tworker, int i) {
  int error;

  error = pthread_create(tworker, NULL, worker, (void *)&i);
  return (seterror(error));
}

// Used to initialize a single dispatcher and set an error if the create fails.
int init_dispatcher(pthread_t *tdispatcher) {
  int error;

  error = pthread_create(tdispatcher, NULL, dispatch, NULL);
  return(seterror(error));
}

// Initializes a request to be used.
request_t init_request(int fd, char *filename) {
  request_t request;
  request.fd = fd;
  request.request = strdup(filename);
  return request;
}

// Used to retrieve a request, takes in a request that will be given what is in the queue
int retrieve_request(request_t *rtemp) {
  int error;
  if(error = pthread_mutex_lock(&buffer_lock)) {
    return error;
  }
  while((total_requests <= 0) && !error) {
    error = pthread_cond_wait(&requests, &buffer_lock);
  }
  if(error) {
    pthread_mutex_unlock(&buffer_lock);
    return error;
  }
  *rtemp = queue_buffer[bufout];
  bufout = (bufout + 1) % queue_length;
  total_requests--;
  if(error = pthread_cond_signal(&slots)) {
    pthread_mutex_unlock(&buffer_lock);
    return error;
  }
  return pthread_mutex_unlock(&buffer_lock);
}

// Used to store the request, takes in a request that will be stored
int store_request(request_t request) {
  int error;
  if(error = pthread_mutex_lock(&buffer_lock)) {
    return error;
  }
  while((total_requests >= queue_length) && !error) {
    error = pthread_cond_wait(&slots, &buffer_lock);
  }
  if(error) {
    pthread_mutex_unlock(&buffer_lock);
    return error;
  }
  queue_buffer[bufin] = request;
  bufin = (bufin + 1) % queue_length;
  total_requests++;
  if(error = pthread_cond_signal(&requests)) {
    pthread_mutex_unlock(&buffer_lock);
    return error;
  }
  return pthread_mutex_unlock(&buffer_lock);
}

/* ************************ Dynamic Pool Code ***********************************/
// Extra Credit: This function implements the policy to change the worker thread pool dynamically
// depending on the number of requests
void* dynamic_pool_size_update(void *arg) {
  printf("Not implemented\n");
}

/**********************************************************************************/

/* ************************************ Cache Code ********************************/

// Function to check whether the given request is present in cache
int getCacheIndex(char *request){
  /// return the index if the request is present in the cache
  for (int i = 0; i < cache_size; i++) {
    if(cache_buffer[i].request == NULL) {
      // This slot has not been filled therefore we can insert into it.
      return -1;
    }
    if(strcmp(cache_buffer[i].request, request) == 0){
      // Request is found so return the index it was found at.
      return i;
    }
  }
  // This is for once the cache has wrapped back around.
  return -1;
}

// Function to add the request and its file content into the cache
void addIntoCache(cache_entry_t new_entry) {
  // Use a static so that it is persistent across calls
  static int cache_slot = 0;
  // This if will check for something being in the slot we are looking at
  if (cache_buffer[cache_slot].len) {
    // Free the two things that are calloc'd elsewhere
    free(cache_buffer[cache_slot].request);
    free(cache_buffer[cache_slot].content);
    // Insert into the cache
    cache_buffer[cache_slot] = new_entry;
    // Increment and rollover if we have hit the max
    cache_slot = (cache_slot + 1) % cache_size;
  } else {
    // Insert into the cache
    cache_buffer[cache_slot] = new_entry;
    // Increment and rollover if we have hit the max
    cache_slot = (cache_slot + 1) % cache_size;
  }
}

// clear the memory allocated to the cache
void deleteCache(){
  for(int i = 0; i < cache_size; i++) {
    // Free individual pieces that where calloc'd
    free(cache_buffer[i].request);
    free(cache_buffer[i].content);
  }
  // Free the entire buffer last
  free(cache_buffer);
}

// Function to initialize the cache
void initCache(){
  // Allocate the cache_buffer array
  cache_buffer = (cache_entry_t *)calloc(cache_size, sizeof(cache_entry_t));
}

// Function to open and read the file from the disk into the memory
// Take in the request and a new_entry to be inserted into the cache
int readFromDisk(char *request, cache_entry_t *new_entry) {
  int stat_err;
  char *path;
  struct stat stat_buf;
  FILE *f;

  // Allocate memory for the path
  path = (char *)calloc(BUFF_SIZE, sizeof(char));
  // Concat a . so that we can access the path relative
  strcat(path, ".");
  // Concat the request on to the path
  strcat(path, request);
  // Open the file for writing
  f = fopen(path, "r");
  if (f == NULL) {
    // If the file is null we have encountered a problem, return
    free(path);
    return 1;
  }
  // Check the stat to find the files size
  stat_err = stat(path, &stat_buf);
  if(stat_err) {
    // If stat is not zero we have a problem, return
    free(path);
    return 1;
  }
  // Allocate the new_entry for space for the file contentss
  new_entry->content = (char *)calloc(stat_buf.st_size, sizeof(char));
  // Allocate the new_entry for space for the request
  new_entry->request = (char *)calloc(BUFF_SIZE, sizeof(char));
  // Read the file contents into our new entry
  fread(new_entry->content, stat_buf.st_size, 1, f);
  // Copy the request over into our new entry
  strcpy(new_entry->request, request);
  // Grab the size for the new entry
  new_entry->len = stat_buf.st_size;
  // Close our file
  fclose(f);
  // Add the new entry into the cache
  addIntoCache(*new_entry);
  // Free the path
  free(path);
  return 0;
}

/**********************************************************************************/

/* ************************************ Utilities ********************************/
// Function to get the content type from the request
char* getContentType(char *mybuf) {
  // Grab the length of our buffer
  int path_len = strlen(mybuf);
  // Allocate memory for the type of content string
  char *content_type = (char *)calloc(13, sizeof(char));

  if (path_len > 5 && strcmp(mybuf + path_len - 5, ".html") == 0) {
    // file type is 'text/html'
    strcpy(content_type, "text/html");
  } else if (path_len > 4 && strcmp(mybuf + path_len - 4, ".jpg") == 0) {
    // file type is 'image/jpeg'
    strcpy(content_type, "image/jpeg");
  } else if (path_len > 4 && strcmp(mybuf + path_len - 4, ".gif") == 0) {
    // file type is 'image/gif'
    strcpy(content_type, "image/gif");
  } else {
    // file type is 'text/plain'
    strcpy(content_type, "text/plain");
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
void * dispatch(void *arg) {
  int error;
  int fd = -1;
  int local_done = 0;
  char filebuf[BUFF_SIZE];
  request_t request;
  while (1) {
    // Accept client connection
    while(fd < 0) {
      fd = accept_connection();
    }

    // Get request from the client
    if(get_request(fd, filebuf) != 0) {
      printf("Failed to get request\n");
    }

    //Create a new request
    request = init_request(fd, filebuf);

    // Add the request into the queue.
    if(error = store_request(request)) {
      break;
    }
    // Reset the fd to -1
    fd = -1;
  }
  return NULL;
}

/**********************************************************************************/

// Function to retrieve the request from the queue, process it and then return a result to the client
void * worker(void *arg) {
  int error;
  int pthread_num = *(int *) arg;
  int requests_handled = 0;
  int ms_time = 0;
  int log_fd = 0;
  int index = 0;
  char *content_type;
  char log[BUFF_SIZE];
  request_t next_request;
  cache_entry_t new_entry;

  while (1) {
    // Retrieve a request from the queue
    if(error = retrieve_request(&next_request)) {
      break;
    }
    // Start recording time.
    ms_time = getCurrentTimeInMillis();
    // Get the content_type from the request
    content_type = getContentType(next_request.request);
    // Check if the request is in the cache
    index = getCacheIndex(next_request.request);
    // If index is -1 need to readFromDisk
    if(index < 0) {
      if(error = readFromDisk(next_request.request, &new_entry)) {
        // Stop recording time
        ms_time = (getCurrentTimeInMillis() - ms_time);
        // Increase requests handled
        requests_handled++;
        // Create the string for the log
        sprintf(log, "[%d][%d][%d][%s][%s][%d us][MISS]\n", pthread_num, requests_handled, next_request.fd, (char *)next_request.request, strerror(errno), ms_time);
        // Print the log to the terminal
        printf("%s\n", log);
        // Return an error
        if(error = return_error(next_request.fd, strerror(errno))) {
          printf("Failed to return an error\n");
        }
        // Reset errno
        errno = 0;
      } else {
        // Stop recording time
        ms_time = (getCurrentTimeInMillis() - ms_time);
        // Increase requests handled
        requests_handled++;
        // Create the string for the log
        sprintf(log, "[%d][%d][%d][%s][%d][%d us][MISS]\n", pthread_num, requests_handled, next_request.fd, (char *)new_entry.request, new_entry.len, ms_time);
        // Print the log to the terminal
        printf("%s\n", log);
        // Return the result
        if(error = return_result(next_request.fd, content_type, new_entry.content, new_entry.len)) {
          printf("Failed to return the request\n");
        }
      }
    } else {
      // Stop recording time
      ms_time = (getCurrentTimeInMillis() - ms_time);
      // Increase requests handled
      requests_handled++;
      // Create the string for the log
      sprintf(log, "[%d][%d][%d][%s][%d][%d us][HIT]\n", pthread_num, requests_handled, next_request.fd, cache_buffer[index].request, cache_buffer[index].len, ms_time);
      // Print the log to the terminal
      printf("%s\n", log);
      // Return the result
      if(error = return_result(next_request.fd, content_type, cache_buffer[index].content, cache_buffer[index].len)) {
        printf("Failed to return the request\n");
      }
    }
    // Reset the time to 0
    ms_time = 0;
    // Close the fd that has been serviced
    close(next_request.fd);
    // Free up the content type string
    free(content_type);
    // Open the log for writing to the end, create it if it doesn't exist
    log_fd = open("web_server_log", O_APPEND | O_CREAT | O_WRONLY, 0777);
    if(log_fd < 0) {
      // If unable to open the log for some reason
      perror("\n");
      printf("Error setting up log\n");
    }
    if(write(log_fd, log, strlen(log)) < strlen(log)) {
      // If unable to write to the log for some reason
      printf("Soemthing went wrong when writing to log\n");
    }
    // Close the log fd
    close(log_fd);
  }
  return NULL;
}

/**********************************************************************************/
// TODO: Finish error checking and such. Chase & Brian
int main(int argc, char **argv) {
  int port = 0;
  int num_workers = 0;
  int num_dispatchers = 0;
  int dynamic_flag = 0;
  int error = 0;
  char *path;
  pthread_t *worker_arr;
  pthread_t *dispatcher_arr;

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

  if(dynamic_flag == 1) {
    printf("Dynamic thread pool not implemented\n");
  }

  queue_length = atoi(argv[6]);
  // Make sure the given queue size is <= maximum
  if (queue_length > MAX_QUEUE_LEN) {
    printf("Queue length exceeds maximum (%d).\n", MAX_QUEUE_LEN);
    return -1;
  }

  cache_size = atoi(argv[7]);
  // Make sure the given cache size is <= maximum
  if (cache_size > MAX_CE) {
    printf("Cache size exceeds maximum (%d).\n", MAX_CE);
    return -1;
  }
  // --------------------------END OF ARGUMENT SETUP AND ERROR CHECKING-------------------------- //


  // Initialize server on port from command line
  init(port);

  // Change the current working directory to server root directory
  chdir(path);

  // Start the server and initialize cache
  printf("Starting server on port %d: %d dispatchers, %d workers\n", port, num_dispatchers, num_workers);

  // Initialize the cache
  initCache();

  // Create the queue array (bounded buffer)
  queue_buffer = (request_t *)calloc(queue_length, sizeof(request_t));

  // Initialize the worker array
  worker_arr = (pthread_t *)calloc(num_workers, sizeof(pthread_t));
  if(worker_arr == NULL) {
    perror("Failed to allocate space for the workers\n");
    return -1;
  }

  // Initialzie the dispatcher array
  dispatcher_arr = (pthread_t *)calloc(num_dispatchers, sizeof(pthread_t));
  if(dispatcher_arr == NULL) {
    perror("Failed to allocate space for the dispatchers\n");
    return -1;
  }

  // Create each worker thread
  for (int i = 0; i < num_workers; i++) {
    if (error = init_worker(worker_arr+i, i)) {
      fprintf(stderr, "Failed to create worker %d:%s\n", i, strerror(error));
      return 1;
    }
  }

  // Create each dispatcher thread
  for (int i = 0; i < num_dispatchers; i++) {
    if (error = init_dispatcher(dispatcher_arr+i)) {
      fprintf(stderr, "Failed to create dispatcher %d:%s\n", i, strerror(error));
      return 1;
    }
  }

  // Join each worker thread
  for(int i = 0; i < num_workers; i++) {
    if(error = pthread_join(worker_arr[i], NULL)) {
      fprintf(stderr, "Failed worker %d join:%s\n", i, strerror(error));
      return 1;
    }
  }

  // Join each dispatcher thread
  for(int i = 0; i < num_dispatchers; i++) {
    if(error = pthread_join(dispatcher_arr[i], NULL)) {
      fprintf(stderr, "Failed dispatcher %d join:%s\n", i, strerror(error));
      return 1;
    }
  }

  // Clean up
  free(worker_arr);
  free(dispatcher_arr);
  free(queue_buffer);
  deleteCache();
  pthread_cond_destroy(&slots);
  pthread_cond_destroy(&requests);
  pthread_mutex_destroy(&error_lock);
  pthread_mutex_destroy(&buffer_lock);
  pthread_mutex_destroy(&read_lock);
  return 0;
}

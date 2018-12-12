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
static int done_flag = 0;
static int cache_size = 0;
static int queue_length = 0;

pthread_mutex_t error_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t buffer_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;
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

//
int retrieve_request(request_t *rtemp) {
  int error;
  if(error = pthread_mutex_lock(&buffer_lock)) {
    return error;
  }
  while((total_requests <= 0) && !error && !done_flag) {
    error = pthread_cond_wait(&requests, &buffer_lock);
  }
  if(error) {
    pthread_mutex_unlock(&buffer_lock);
    return error;
  }
  if(done_flag && (total_requests <= 0)) {
    pthread_mutex_unlock(&buffer_lock);
    return ECANCELED;
  }
  *rtemp = queue_buffer[bufout];
  bufout = (bufout + 1) % queue_length;
  total_requests--;
  if(error = pthread_cond_signal(&slots)) {
    pthread_mutex_unlock(&buffer_lock);
  }
  return pthread_mutex_unlock(&buffer_lock);
}

int store_request(request_t request) {
  int error;
  if(error = pthread_mutex_lock(&buffer_lock)) {
    return error;
  }
  while((total_requests >= queue_length) && !error && !done_flag) {
    error = pthread_cond_wait(&slots, &buffer_lock);
  }
  if(error) {
    pthread_mutex_unlock(&buffer_lock);
    return error;
  }
  if(done_flag) {
    pthread_mutex_unlock(&buffer_lock);
    return ECANCELED;
  }
  queue_buffer[bufin] = request;
  bufin = (bufin + 1) % queue_length;
  total_requests++;
  if(error = pthread_cond_signal(&slots)) {
    pthread_mutex_unlock(&buffer_lock);
    return error;
  }
  return pthread_mutex_unlock(&buffer_lock);
}

int getdone(int *flag) {
  int error;
  if (error = pthread_mutex_lock(&buffer_lock)) {
    return error;
  }
  *flag = done_flag;
  return pthread_mutex_unlock(&buffer_lock);
}

int setdone() {
  int error1;
  int error2;
  int error3;

  if(error1 = pthread_mutex_lock(&buffer_lock)) {
    return error1;
  }
  done_flag = 1;
  error1 = pthread_cond_broadcast(&requests);
  error2 = pthread_cond_broadcast(&slots);
  error3 = pthread_mutex_unlock(&buffer_lock);
  if(error1) {
    return error1;
  }
  if(error2) {
    return error2;
  }
  if(error3) {
    return error3;
  }
  return 0;
}

/* ************************ Dynamic Pool Code ***********************************/
// Extra Credit: This function implements the policy to change the worker thread pool dynamically
// depending on the number of requests
// TODO: If we have time for the extra credit
void* dynamic_pool_size_update(void *arg) {
  printf("Not implemented\n");
}

/**********************************************************************************/

/* ************************************ Cache Code ********************************/

// Function to check whether the given request is present in cache
// TODO: Jared
int getCacheIndex(char *request){
  /// return the index if the request is present in the cache
  for (int i = 0; i < cache_size; i++) {
    if(cache_buffer[i].request == NULL) {
      return -1;
    }
    if(strcmp(cache_buffer[i].request, request) == 0){
      return i;
    }
  }
}

// Function to add the request and its file content into the cache
// TODO: Jared
void addIntoCache(char *request, char *file , int memory_size){
  static int cache_slot = 0;
  // It should add the request at an index according to the cache replacement policy
  // Make sure to allocate/free memory when adding or replacing cache entries
  /*
    if = cache at max size print "its full bud"
    else = goes into a check if pointer is present
  */
  printf("|%d|\n", cache_slot);
  if (cache_buffer[cache_slot].len != 0){
    /*
      if pointer is present we free up that slot
      then we place our new entry into that slot
      increment the size of cache
    */
    printf("|%d|\n", cache_slot);
    cache_buffer[cache_slot].request = realloc(cache_buffer[cache_slot].request, BUFF_SIZE * sizeof(char));
    cache_buffer[cache_slot].content = realloc(cache_buffer[cache_slot].content, memory_size * sizeof(char));
    memcpy(cache_buffer[cache_slot].request, request, BUFF_SIZE);
    memcpy(cache_buffer[cache_slot].content, file, memory_size);
    cache_buffer[cache_slot].len = memory_size;
    printf("cache_buffer|%d|->request: |%s|", cache_slot, cache_buffer[cache_slot].content);
    printf("cache_buffer[%d]->len: |%d|\n", cache_slot, cache_buffer[cache_slot].len);
    cache_slot = (cache_slot + 1) % cache_size;
  } else {
    /*
      if no pointer is found then we just place the
      pointer to struct into the slot
    */
    cache_buffer[cache_slot].request = realloc(cache_buffer[cache_slot].request, BUFF_SIZE * sizeof(char));
    cache_buffer[cache_slot].content = realloc(cache_buffer[cache_slot].content, memory_size * sizeof(char));
    memcpy(cache_buffer[cache_slot].request, request, BUFF_SIZE);
    memcpy(cache_buffer[cache_slot].content, file, memory_size);
    cache_buffer[cache_slot].len = memory_size;
    printf("cache_buffer[%d]->len: |%d|\n", cache_slot, cache_buffer[cache_slot].len);
    cache_slot = (cache_slot + 1) % cache_size;
  }
}

// clear the memory allocated to the cache
// TODO: Jared
void deleteCache(){
  // De-allocate/free the cache memory
  // frees the pointers within the pointer array first
  // then we free the pointer array
  free(cache_buffer);
}

// Function to initialize the cache
// TODO: Jared
void initCache(){
  // Allocating memory and initializing the cache array
  // creates an array of pointers, which these pointers point to structs
  cache_buffer = (cache_entry_t *)calloc(cache_size, sizeof(cache_entry_t));
}

// Function to open and read the file from the disk into the memory
// Add necessary arguments as needed
// TODO: Brian
int readFromDisk(char *request, struct stat *stat_buff, char **buf) {
  FILE *f;
  int stat_err;
  char *path = malloc(sizeof(char) * BUFF_SIZE + 1);
  // Open and read the contents of file given the request
  strcat(path, ".");
  strcat(path, request);
  f = fopen(path, "r");
  if (f == NULL) {
    free(path);
    return -1;
  }
  stat_err = stat(path, stat_buff);
  if(stat_err != 0) {
    free(path);
    return -1;
  }
  (*buf) = (char *)malloc(stat_buff->st_size * sizeof(char) + 1);
  fread(*buf, stat_buff->st_size, 1, f);
  fclose(f);
  free(path);
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
  int localdone = 0;
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

    //Createa a new request
    request = init_request(fd, filebuf);

    // Make sure you created a valid request
    if(request.fd != fd) {
      break;
    }
    while(!localdone) {
      // Add the request into the queue.
      if(error = store_request(request)) {
        break;
      }
      // Check to see if the request storage is done.
      if(error = getdone(&localdone)) {
        break;
      }
      // Set the globabl done flag.
      if(error = setdone()) {
        fprintf(stderr, "Failed to set done indicator:%s\n", strerror(error));
        break;
      }
    }
    if(error != ECANCELED) {
      seterror(error);
    }
    fd = -1;
  }
  return NULL;
}

/**********************************************************************************/

// Function to retrieve the request from the queue, process it and then return a result to the client
// TODO: Chase
void * worker(void *arg) {
  int error;
  int pthread_num = *(int *) arg;
  int requests_handled = 0;
  int ms_time = 0;
  int fd = 0;
  int log_fd = 0;
  int index = 0;
  char *request;
  char *content_type;
  char log[BUFF_SIZE];
  request_t next_request;

  while (1) {
    while(error = retrieve_request(&next_request)) {
      printf("Haven't gotten request yet\n");
    }
    printf("next_request.fd: |%d|\n", next_request.fd);
    printf("next_request.request: |%s|\n", (char *)next_request.request);
    // Start recording time
    ms_time = getCurrentTimeInMillis();
    // Get the request from the queue

    // Get the data from the disk or the cache
    index = getCacheIndex(request);
    content_type = getContentType(request);
    if(index < 0) {
      struct stat stat_buff;
      char *file;
      int read_err = readFromDisk(request, &stat_buff, &file);
      if(read_err < 0) {
        ms_time = (getCurrentTimeInMillis() - ms_time);
        requests_handled++;
        sprintf(log, "[%d][%d][%d][%s][%d][%d us][MISS]\n", pthread_num, requests_handled, fd, request, errno, ms_time);
        printf("%s", log);
        if(return_error(fd, strerror(errno)) != 0) {
          printf("Failed to return an error\n");
        }
        errno = 0;
        free(file);
        free(content_type);
      } else {
        addIntoCache(request, file, stat_buff.st_size);
        // Stop recording the time
        ms_time = (getCurrentTimeInMillis() - ms_time);
        requests_handled++;
        sprintf(log, "[%d][%d][%d][%s][%ld][%d us][MISS]\n", pthread_num, requests_handled, fd, request, stat_buff.st_size, ms_time);
        printf("%s", log);
        // Log the request into the file and terminal
        // return the result
        if(return_result(fd, content_type, file, stat_buff.st_size) != 0) {
          printf("Failed to return a result\n");
        }
        free(file);
        free(content_type);
      }
    } else {
      // Stop recording the time
      ms_time = (getCurrentTimeInMillis() - ms_time);
      requests_handled++;
      int cached_size = cache_buffer[index].len;
      char *cached_content = malloc(cached_size * sizeof(char));
      memcpy(cached_content, cache_buffer[index].content, cached_size);
      sprintf(log, "[%d][%d][%d][%s][%d][%d us][HIT]\n", pthread_num, requests_handled, fd, request, cached_size, ms_time);
      printf("%s", log);
      // Log the request into the file and terminal

      // return the result
      if(return_result(fd, content_type, cached_content, cached_size) != 0) {
        printf("Failed to return a cached result\n");
      }
      free(cached_content);
      free(content_type);
    }
    free(request);
    log_fd = open("web_server_log", O_APPEND | O_CREAT | O_WRONLY, 0777);
    if(log_fd < 0) {
      perror("\n");
      printf("Error setting up log\n");
    }
    if(write(log_fd, log, strlen(log)) < strlen(log)) {
      printf("Something went wrong when writing to log\n");
    }
    close(log_fd);
    if(pthread_mutex_unlock(&queue_lock) < 0) {
      printf("Failed to lock queue mutex\n");
    }
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

  // if caching is enabled, generate cache
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
  initCache();
  // Create the queue array (bounded buffer)
  queue_buffer = (request_t *)calloc(queue_length, sizeof(request_t));
  // Initialize arrays for both thread types
  worker_arr = (pthread_t *)calloc(num_workers, sizeof(pthread_t));
  if(worker_arr == NULL) {
    perror("Failed to allocate space for the workers\n");
    return -1;
  }

  dispatcher_arr = (pthread_t *)calloc(num_dispatchers, sizeof(pthread_t));
  if(dispatcher_arr == NULL) {
    perror("Failed to allocate space for the dispatchers\n");
    return -1;
  }

  for (int i = 0; i < num_workers; i++) {
    if (error = init_worker(worker_arr+i, i)) {
      fprintf(stderr, "Failed to create worker %d:%s\n", i, strerror(error));
      return 1;
    }
  }

  for (int i = 0; i < num_dispatchers; i++) {
    if (error = init_dispatcher(dispatcher_arr+i)) {
      fprintf(stderr, "Failed to create dispatcher %d:%s\n", i, strerror(error));
      return 1;
    }
  }

  for(int i = 0; i < num_workers; i++) {
    if(error = pthread_join(worker_arr[i], NULL)) {
      fprintf(stderr, "Failed worker %d join:%s\n", i, strerror(error));
      return 1;
    }
  }

  for(int i = 0; i < num_dispatchers; i++) {
    if(error = pthread_join(dispatcher_arr[i], NULL)) {
      fprintf(stderr, "Failed dispatcher %d join:%s\n", i, strerror(error));
      return 1;
    }
  }

  // // Clean up
  // free(worker_arr);
  // free(dispatcher_arr);
  // deleteQueue();
  // deleteCache();
  // probably need to free threads, queue, cache
  // and destroy locks
  return 0;
}

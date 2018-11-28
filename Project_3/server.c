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

// defines:
#define MAX_THREADS 100
#define MAX_QUEUE_LEN 100
#define MAX_CE 100
#define INVALID -1
#define BUFF_SIZE 1024

// global declarations:
int port;
char path[1024]; // change?
int num_dispatchers;
int num_workers;
int dynamic_flag;
int queue_length;
int cache_size;

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

/* ************************ Dynamic Pool Code ***********************************/
// Extra Credit: This function implements the policy to change the worker thread pool dynamically
// depending on the number of requests
void * dynamic_pool_size_update(void *arg) {
  while(1) {
    // Run at regular intervals
    // Increase / decrease dynamically based on your policy
  }
}
/**********************************************************************************/

/* ************************************ Cache Code ********************************/

// Function to check whether the given request is present in cache
int getCacheIndex(char *request){
  /// return the index if the request is present in the cache
}

// Function to add the request and its file content into the cache
void addIntoCache(char *mybuf, char *memory , int memory_size){
  // It should add the request at an index according to the cache replacement policy
  // Make sure to allocate/free memeory when adding or replacing cache entries
}

// clear the memory allocated to the cache
void deleteCache(){
  // De-allocate/free the cache memory
}

// Function to initialize the cache
void initCache(){
  // Allocating memory and initializing the cache array
}

// Function to open and read the file from the disk into the memory
// Add necessary arguments as needed
int readFromDisk(/*necessary arguments*/) {
  // Open and read the contents of file given the request
}

/**********************************************************************************/

/* ************************************ Utilities ********************************/
// Function to get the content type from the request
char* getContentType(char * mybuf) {
  // Should return the content type based on the file type in the request
  // (See Section 5 in Project description for more details)
}

// This function returns the current time in milliseconds
int getCurrentTimeInMills() {
  struct timeval curr_time;
  gettimeofday(&curr_time, NULL);
  return curr_time.tv_usec;
}

/**********************************************************************************/

// Function to receive the request from the client and add to the queue
void * dispatch(void *arg) {

  while (1) {

    // Accept client connection

    // Get request from the client

    // Add the request into the queue

   }
   return NULL;
}

/**********************************************************************************/

// Function to retrieve the request from the queue, process it and then return a result to the client
void * worker(void *arg) {

   while (1) {

    // Start recording time

    // Get the request from the queue

    // Get the data from the disk or the cache

    // Stop recording the time

    // Log the request into the file and terminal

    // return the result
  }
  return NULL;
}

/**********************************************************************************/

int main(int argc, char **argv) {

  // Error check on number of arguments
  // Decide to check if caching is enabled [argc == 8 -> Caching enabled]
  if(argc != 7 && argc != 8) {
    printf("usage: %s port path num_dispatchers num_workers dynamic_flag queue_length cache_size\n", argv[0]);
    return -1;
  }

  // Get the input args and error check on them
  port = atoi(argv[1]);
  if (port < 1025 || port > 65535) { // not sure if this check is needed
    printf("Port out of range, must be between 1025 and 65535.\n");
    return -1;
  }

  path = argv[2]; // use strncpy?
  // error check on path?

  num_dispatchers = atoi(argv[3]);
  if (num_dispatchers <= 0 || num_dispatchers > MAX_THREADS) {
    printf("Invalid number of dispatcher threads. Please enter a number between 0 and %d.\n", MAX_THREADS);
    return -1;
  }

  num_workers = atoi(argv[4]);
  if (num_workers <= 0 || num_workers > MAX_THREADS) {
    printf("Invalid number of worker threads. Please enter a number between 0 and %d.\n", MAX_THREADS);
    return -1;
  }

  // Make sure the total number of threads is <= 100
  if ((num_dispatchers + num_workers) > MAX_THREADS) {
    printf("Number of threads exceeds maximum (%d).\n", MAX_THREADS);
    return -1;
  }

  dynamic_flag = atoi(argv[5]);
  // error check on dynamic_flag?

  queue_length = atoi(argv[6]);
  // Make sure the given queue size is <= 100
  if (queue_length > MAX_QUEUE_LEN) {
    printf("Queue length exceeds maximum (%d).\n", MAX_QUEUE_LEN);
    return -1;
  }

  // if caching is enabled, generate cache
  if (argc == 8) {
    cache_size = atoi(argv[7]);
    // Make sure the given cache size is <= 100
    if (cache_size > MAX_CE) {
      printf("Cache size exceeds maximum (%d).\n", MAX_CE);
      return -1;
    }
  } else {
    cache_size = 0;
  }



  // TODO error check on dynamic flag?





  // Initialize server on port from command line
  init(port);

  // Change the current working directory to server root directory
  chdir(path);

  // Start the server and initialize cache

  // Create dispatcher and worker threads
  for (int i = 0; i < num_dispatchers; i++) {
    if (pthread_create());
  }

  for (int i = 0; i < num_workers; i++) {

  }

  // Clean up
  // probably need to free threads, queue, cache
  // and destroy locks

  return 0;
}

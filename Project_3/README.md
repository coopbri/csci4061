CSci4061 Fall 2018 Project 3
============================
Name: Jared Erlien, Chase Rogness, Brian Cooper

X500: erlie003, rogne066, coope824

- The program works by using a bounded buffer to store given requests in a fifo queue. The queue has a lock it shares across the two of them to allow us the ability to make sure one does not extract from another. The buffer also has two CV's that alert one or another when they are allowed to continue putting in or take out of the buffer. This allows us to continuously get a request and take one out and service it. Whether this be through the cache or through the reading directly from the disk. Which inside of the worker it is also locked to allow us not to accidentally be overwriting other threads data.
- To compile the program, run the `make` command from a command line interface in the directory containing the source files (the Makefile is provided). This will result in an executable called `web_server`.
To run the program, run the executable in the current directory with `.\web_server port path num_dispatchers num_workers dynamic_flag queue_length cache_size` (change the directory as needed). The final parameter is only needed if you would like to use caching (explained below). The parameters are:

  - `port` (integer): the port to run the server on (range of 1025-65535, inclusive)
  - `path` (string): path to web root location
  - `num_dispatchers` (integer): number of dispatcher threads to initialize
  - `num_workers` (integer): number of worker threads to initialize
  - `dynamic_flag` (integer): indicates static or dynamic worker thread pool size (default 0)
  - `queue_length` (integer): size of request queue
  - `cache_size` (integer): number of entries available in cache

- The program works by (how the program works)
- This program supports caching. The caching mechanism works by adding into the slot indicated by our cache_size variable. We implemented a FIFO policy in order to take care of overflow.
- For worker thread pool size we agreed upon a static number for working on our project. 
- Contributions to the Project are as follows:
  - Chase: Worked on functionality of worker and dispatcher threads. Provided debugging solutions and worked on fixing the memory     leaks.
  - Brian: Worked on the readDisk and getContentType.
  - Jared: Worked on the cache functions and decided upon the FIFO cache policy.

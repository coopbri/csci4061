CSci4061 Fall 2018 Project 3
============================
Name: Jared Erlien, Chase Rogness, Brian Cooper

X500: erlie003, rogne066, coope824

Group Number: 17
___
#### Individual Contributions
- All: Worked on main function error checking and structure. Worked on overall program error checking and memory management. Worked on request handling.
- Jared: Worked on cache functions, cache memory management and cache requests.
- Chase: Worked on worker and dispatcher thread functions. Worked on dynamic flag and mutex lock code.
- Brian: Worked on reading from disk and obtaining file information (such as content type). Worked on stat struct.
___
#### Project Information
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
- This program supports caching. The caching mechanism works by (explanation of caching mechanism used)
- (explanation of policy to dynamically change worker thread pool size)

- Current testing:
- terminal 1: ./web_server 9000 path_to_project3/testing 3 3 0 3 3
- terminal 2: wget -i 127.0.0.1:9000/testurls
- Error occurs due to issue with strcpy from the queue, can print it out but cant seem to copy it over.

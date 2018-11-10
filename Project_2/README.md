/* CSCI4061 F2018 Assignment 2

1. Purpose of program: Represent basic components of a multi-user chat system. Utilizing a server as a medium between separate clients with unique ID's.
2. Who did what:
    Chase: Polished the finished code ironing out bugs majority including pipe handling.
    Brian: Implemented functions for use server and client use.
    Jared: Provided framework for server and client communication
3. How to compile:
    In the directory first run
    '''
    make
    '''
    in order to compile the program which will result in two files called server and client
4. How to user program from the shell:
    On one terminal execute the command
    '''
    ./server
    '''
    to initiate the server and on a separate command line process to start a client do
    '''
    ./client <user_id>
    e.g. ./client Jared
    '''
5. What your program does:
    When users connect it allows them to either broadcast their message to all users connected to the server or send a private message to a user connected to the server.
    It also allows admin control such as listing out all users connected to server, kick <user>, or disconnecting everyone from the server.
6. Explicit assumptions made:
    Some assumptions made were:
    - Users wouldn't input a message longer than 255 characters long
    - Users use an id that isn't longer than 15 characters
    - Max capacity is only 10 slots for the server and there aren't more than 10 people on the server at any given time.
    - Server processes and user processes are stopped manually while disconnects from server are handled.
7. Error handling strategies:
    Some error handling strategies are given that a function returned both a successful int or unsuccessful it allowed us to produce an error message for said input.
    Also
